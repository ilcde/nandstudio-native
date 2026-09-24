// SPDX-License-Identifier: GPL-3.0-or-later
// Native hardware implementation. See docs/hardware.md for compatibility
// limits.
#include "hdl.hpp"
#include "builtin_hdl.hpp"
#include <algorithm>
#include <cctype>
#include <charconv>
#include <deque>
#include <numeric>
#include <set>
namespace nand {
namespace {
struct Token {
  std::string text;
  int line = 1, column = 1;
};
struct Ref {
  std::string name;
  int lo = -1, hi = -1;
  Token token;
};
struct Pin {
  std::string name;
  int width = 1;
  bool input = false;
};
struct Connection {
  Ref left, right;
};
struct Part {
  std::string name;
  std::vector<Connection> connections;
  Token token;
};
struct Declaration {
  std::string name, builtin;
  std::vector<Pin> pins;
  std::vector<Part> parts;
  std::set<std::string> clocked;
};
class Parser {
  std::vector<Token> tokens_;
  std::size_t pos_ = 0;
  const Token &peek() const { return tokens_.at(pos_); }
  [[noreturn]] void fail(const std::string &m) const {
    throw Error(m, peek().line, peek().column);
  }
  bool take(std::string_view s) {
    if (peek().text == s) {
      ++pos_;
      return true;
    }
    return false;
  }
  void need(std::string_view s) {
    if (!take(s))
      fail("Expected '" + std::string(s) + "'");
  }
  std::string identifier() {
    auto t = peek();
    if (t.text.empty() ||
        !(std::isalpha(static_cast<unsigned char>(t.text[0])) ||
          t.text[0] == '_'))
      fail("Identifier expected");
    ++pos_;
    return t.text;
  }
  int number() {
    auto t = peek();
    int value;
    auto r =
        std::from_chars(t.text.data(), t.text.data() + t.text.size(), value);
    if (r.ec != std::errc{} || r.ptr != t.text.data() + t.text.size() ||
        value < 0 || value > 16)
      fail("Pin index or width out of range");
    ++pos_;
    return value;
  }
  Ref reference() {
    Ref r;
    r.token = peek();
    r.name = identifier();
    if (take("[")) {
      r.lo = number();
      r.hi = take("..") ? number() : r.lo;
      need("]");
      if (r.hi < r.lo)
        fail("Invalid sub-bus range");
    }
    return r;
  }

public:
  explicit Parser(std::string_view text) {
    if (text.size() > 32 * 1024 * 1024)
      throw Error("HDL source exceeds 32 MiB limit");
    std::size_t p = 0;
    int line = 1, column = 1;
    auto advance = [&]() {
      auto c = text[p++];
      if (c == '\n') {
        ++line;
        column = 1;
      } else
        ++column;
      return c;
    };
    while (p < text.size()) {
      if (std::isspace(static_cast<unsigned char>(text[p]))) {
        advance();
        continue;
      }
      if (text.substr(p, 2) == "//") {
        while (p < text.size() && text[p] != '\n')
          advance();
        continue;
      }
      if (text.substr(p, 2) == "/*") {
        advance();
        advance();
        while (p < text.size() && text.substr(p, 2) != "*/")
          advance();
        if (p == text.size())
          throw Error("Unterminated comment", line, column);
        advance();
        advance();
        continue;
      }
      Token t{{}, line, column};
      char c = advance();
      t.text += c;
      if (std::isalnum(static_cast<unsigned char>(c)) || c == '_') {
        while (p < text.size() &&
               (std::isalnum(static_cast<unsigned char>(text[p])) ||
                text[p] == '_'))
          t.text += advance();
      } else if (c == '.' && p < text.size() && text[p] == '.')
        t.text += advance();
      else if (std::string_view("{}[](),;:=").find(c) == std::string_view::npos)
        throw Error("Unexpected HDL character", t.line, t.column);
      tokens_.push_back(std::move(t));
      if (tokens_.size() > 1000000)
        throw Error("HDL token limit exceeded");
    }
    tokens_.push_back({"", line, column});
  }
  Declaration parse() {
    Declaration d;
    need("CHIP");
    d.name = identifier();
    need("{");
    std::set<std::string> names;
    while (peek().text == "IN" || peek().text == "OUT") {
      bool input = take("IN");
      if (!input)
        need("OUT");
      do {
        Pin pin;
        pin.input = input;
        pin.name = identifier();
        if (!names.insert(pin.name).second)
          fail("Duplicate pin " + pin.name);
        if (take("[")) {
          pin.width = number();
          need("]");
          if (pin.width == 0)
            fail("Pin width must be positive");
        }
        d.pins.push_back(std::move(pin));
      } while (take(","));
      need(";");
    }
    if (take("BUILTIN")) {
      d.builtin = identifier();
      need(";");
      if (take("CLOCKED")) {
        do {
          auto n = identifier();
          if (!names.contains(n))
            fail("Unknown clocked pin " + n);
          d.clocked.insert(n);
        } while (take(","));
        need(";");
      }
    } else {
      need("PARTS");
      need(":");
      while (peek().text != "}" && !peek().text.empty()) {
        Part part;
        part.token = peek();
        part.name = identifier();
        need("(");
        if (peek().text != ")")
          do {
            auto l = reference();
            need("=");
            auto r = reference();
            part.connections.push_back({l, r});
          } while (take(","));
        need(")");
        need(";");
        d.parts.push_back(std::move(part));
      }
    }
    need("}");
    if (!peek().text.empty())
      fail("Expected end-of-file after '}'");
    return d;
  }
};
using Bus = std::vector<int>;
struct Signal {
  Bus bits;
  std::string direction;
  int node;
};
// Legacy Node stores a short regardless of declared pin width. Only explicit
// sub-bus adapters mask it. Bit connectivity remains useful for scheduling,
// but cannot represent this observable whole-pin behavior.
struct Wire {
  int target;
  int sourceLo = -1, targetLo = -1, width = 0;
};
struct Node {
  Word value = 0;
  std::vector<Wire> listeners;
};
struct Device {
  std::string chip, kind, path;
  std::map<std::string, Signal> pins;
  std::map<std::string, std::string> roles;
  std::vector<std::string> inputs, outputs;
  std::set<std::string> clocked;
  std::vector<Word> memory, lastInputs;
  Word state = 0;
  bool dirty = true;
};
bool inspectableRegister(const std::string &k) {
  return k == "ARegister" || k == "DRegister" || k == "PC";
}
bool reg(const std::string &k) {
  return k == "DFF" || k == "Bit" || k == "Register" || k == "ARegister" ||
         k == "DRegister" || k == "PC";
}
std::size_t memorySize(const std::string &k) {
  static const std::map<std::string, std::size_t> sizes = {
      {"RAM8", 8},       {"RAM64", 64},    {"RAM512", 512},  {"RAM4K", 4096},
      {"RAM16K", 16384}, {"Screen", 8192}, {"ROM32K", 32768}};
  auto i = sizes.find(k);
  return i == sizes.end() ? 0 : i->second;
}
bool supported(const std::string &k) {
  static const std::set<std::string> s = {
      "Nand",      "And",       "Or",     "Xor",       "Not",
      "Not16",     "Mux",       "DMux",   "DMux4Way",  "DMux8Way",
      "Mux4Way16", "Mux8Way16", "Or8Way", "HalfAdder", "FullAdder",
      "Add16",     "Inc16",     "ALU",    "Keyboard"};
  return s.contains(k) || reg(k) || memorySize(k) > 0;
}
int parseIndex(const std::string &s) {
  int n = 0;
  if (s.empty())
    return 0;
  auto r = std::from_chars(s.data(), s.data() + s.size(), n);
  if (r.ec != std::errc{} || r.ptr != s.data() + s.size() || n < 0)
    throw Error("Illegal component index");
  return n;
}
} // namespace
struct Hardware::Impl {
  std::string name;
  std::vector<int> parent;
  std::vector<Node> nodes;
  std::map<std::string, Signal> rootPins;
  std::vector<Device> devices;
  struct Instance {std::string path, chip; bool builtin; std::map<std::string, Signal> pins;};
  std::vector<Instance> instances;
  std::map<int, std::string> nodeNames{{0,"false"},{1,"true"},{2,"clk"}};
  std::vector<int> order;
  std::map<std::string, Declaration> declarations;
  std::vector<std::string> loading;
  std::uint64_t time = 0;
  bool up = false;
  std::size_t memoryWords = 0;
  int net() {
    if (parent.size() >= 2000000)
      throw Error("HDL connection resource limit exceeded");
    int n = int(parent.size());
    parent.push_back(n);
    return n;
  }
  int find(int n) {
    int r = n;
    while (parent[r] != r)
      r = parent[r];
    while (parent[n] != n) {
      int old = parent[n];
      parent[n] = r;
      n = old;
    }
    return r;
  }
  int find(int n) const {
    while (parent[n] != n)
      n = parent[n];
    return n;
  }
  void join(int a, int b) {
    a = find(a);
    b = find(b);
    if (a == b)
      return;
    if (a < 3 && b < 3)
      throw Error("Conflicting constant drivers");
    if (b < 3)
      std::swap(a, b);
    parent[b] = a;
  }
  Bus bus(int width) {
    Bus b;
    for (int i = 0; i < width; ++i)
      b.push_back(net());
    return b;
  }
  Signal signal(int width, const std::string &direction) {
    auto bits = bus(width);
    int id = int(nodes.size());
    nodes.emplace_back();
    return {std::move(bits), direction, id};
  }
  Word read(const Signal &s) const { return nodes.at(s.node).value; }
  void writeNode(int id, Word value) {
    struct Change { int node; Word value; int lo; int width; };
    std::deque<Change> pending{{id, value, -1, 0}};
    std::size_t changes = 0;
    while (!pending.empty()) {
      auto [n, v, lo, width] = pending.front();
      pending.pop_front();
      auto &node = nodes.at(n);
      if (lo >= 0) {
        const auto mask = (1u << width) - 1u;
        v = Word((node.value & ~(mask << lo)) |
                 ((std::uint32_t(v) & mask) << lo));
      }
      if (node.value == v)
        continue;
      if (++changes > 4000000)
        throw Error("HDL wire propagation resource limit exceeded");
      node.value = v;
      for (auto &wire : node.listeners) {
        const auto mask = (1u << wire.width) - 1u;
        Word next = wire.sourceLo < 0 ? v : Word((v >> wire.sourceLo) & mask);
        pending.push_back({wire.target, next, wire.targetLo, wire.width});
      }
    }
  }
  void write(const Signal &s, Word v) { writeNode(s.node, v); }
  void connect(int source, int target, int sourceLo, int targetLo, int width) {
    nodes.at(source).listeners.push_back({target, sourceLo, targetLo, width});
  }
  Declaration declaration(const std::string &chip,
                          const HdlResolver &resolver) {
    if (declarations.contains(chip))
      return declarations.at(chip);
    auto source = resolver(chip);
    if (!source)
      for (auto [n, text] : builtinHdl)
        if (n == chip) {
          source = std::string(text);
          break;
        }
    if (!source)
      throw Error("Cannot find HDL chip " + chip);
    Declaration d;
    try {
      d = Parser(*source).parse();
    } catch (Error &e) {
      if (e.file.empty())
        e.file = chip + ".hdl";
      throw;
    }
    if (d.name != chip)
      throw Error("CHIP name does not match file name", 1, 1, chip + ".hdl");
    declarations.emplace(chip, d);
    return d;
  }
  Bus slice(const Signal &s, const Ref &r) {
    if (r.lo < 0)
      return s.bits;
    if (r.hi >= int(s.bits.size()))
      throw Error(r.name + ": the specified sub bus is not in the bus range",
                  r.token.line, r.token.column);
    return Bus(s.bits.begin() + r.lo, s.bits.begin() + r.hi + 1);
  }
  std::map<std::string, Signal>
  instantiate(const std::string &chip, const std::string &path,
              const HdlResolver &resolver,
              const std::function<bool()> &cancelled) {
    if (cancelled && cancelled())
      throw Error("Cancelled");
    if (loading.size() >= 128 ||
        std::find(loading.begin(), loading.end(), chip) != loading.end())
      throw Error("Recursive HDL dependency: " + chip);
    loading.push_back(chip);
    auto d = declaration(chip, resolver);
    std::map<std::string, Signal> signals;
    for (const auto &p : d.pins)
      signals.emplace(p.name,
                      signal(p.width, p.input ? "input" : "output"));
    if (!d.builtin.empty()) {
      if (!supported(d.builtin))
        throw Error("Unsupported legacy built-in extension " + d.builtin, 1, 1,
                    chip + ".hdl");
      if (devices.size() >= 100000)
        throw Error("HDL device resource limit exceeded");
      Device device;
      device.chip = chip;
      device.kind = d.builtin;
      device.path = path;
      device.pins = signals;
      device.clocked = d.clocked;
      for (auto &p : d.pins)
        (p.input ? device.inputs : device.outputs).push_back(p.name);
      // Java built-ins bind by input/output position, not HDL pin spelling.
      // Use the bundled signature solely to name those positional roles.
      for (auto [builtinName, source] : builtinHdl)
        if (builtinName == d.builtin) {
          auto signature = Parser(source).parse();
          std::size_t inIndex = 0, outIndex = 0;
          for (auto &pin : signature.pins) {
            auto &names = pin.input ? device.inputs : device.outputs;
            auto &index = pin.input ? inIndex : outIndex;
            if (index >= names.size())
              throw Error("Too few pins for built-in " + d.builtin);
            device.roles[pin.name] = names[index++];
          }
          break;
        }
      memoryWords += memorySize(device.kind);
      if (memoryWords > 16 * 1024 * 1024)
        throw Error("HDL memory resource limit exceeded");
      device.memory.resize(memorySize(device.kind));
      devices.push_back(std::move(device));
    } else {
      std::map<std::string, std::vector<bool>> driven;
      std::map<std::string, int> ordinals;
      for (auto &part : d.parts) {
        auto child =
            instantiate(part.name,
                        path + "/" + part.name + "[" +
                            std::to_string(ordinals[part.name]++) + "]",
                        resolver, cancelled);
        for (auto &c : part.connections) {
          auto l = child.find(c.left.name);
          if (l == child.end() || l->second.direction == "internal")
            throw Error(c.left.name + " is not a pin in " + part.name,
                        c.left.token.line, c.left.token.column, chip + ".hdl");
          auto lb = slice(l->second, c.left);
          Bus rb;
          int rightNode;
          int sourceLo = c.right.lo;
          auto &rn = c.right.name;
          bool input = l->second.direction == "input";
          if (rn == "true" || rn == "false" || rn == "clk") {
            if (!input)
              throw Error("Cannot drive a constant", c.right.token.line,
                          c.right.token.column);
            if (c.right.lo >= 0)
              throw Error("Special node may not be subscripted",
                          c.right.token.line, c.right.token.column);
            if (rn == "clk" && lb.size() != 1)
              throw Error("Clock width must be one");
            rb.assign(lb.size(), rn == "true" ? 1 : rn == "false" ? 0 : 2);
            rightNode = rn == "true" ? 1 : rn == "false" ? 0 : 2;
            sourceLo = 0; // Special true is narrowed to the connected width.
          } else {
            if (!signals.contains(rn))
              signals.emplace(rn, signal(int(lb.size()), "internal"));
            auto &right = signals.at(rn);
            rightNode = right.node;
            if (right.direction == "internal" && c.right.lo >= 0)
              throw Error(rn + ": sub bus of an internal node may not be used",
                          c.right.token.line, c.right.token.column);
            rb = slice(right, c.right);
            if (input && right.direction == "output")
              throw Error("Can't connect gate's output pin to part",
                          c.right.token.line, c.right.token.column);
            if (!input && right.direction == "input")
              throw Error("Can't connect part's output pin to gate's input pin",
                          c.right.token.line, c.right.token.column);
            if (!input) {
              auto &bits = driven[rn];
              bits.resize(right.bits.size());
              int start = c.right.lo < 0 ? 0 : c.right.lo;
              for (std::size_t i = 0; i < rb.size(); ++i) {
                if (bits[start + i])
                  throw Error(
                      "A pin may only be fed once by a part's output pin",
                      c.right.token.line, c.right.token.column);
                bits[start + i] = true;
              }
            }
          }
          if (lb.size() != rb.size())
            throw Error(c.left.name + " and " + rn +
                            " have different bus widths",
                        c.right.token.line, c.right.token.column);
          for (std::size_t i = 0; i < lb.size(); ++i)
            join(lb[i], rb[i]);
          if (input)
            connect(rightNode, l->second.node, sourceLo, c.left.lo, int(lb.size()));
          else
            connect(l->second.node, rightNode, c.left.lo, c.right.lo, int(lb.size()));
        }
      }
      for (auto &[n, s] : signals)
        if (s.direction == "internal" && !driven.contains(n))
          throw Error(n + " has no source pin", 1, 1, chip + ".hdl");
    }
    for (auto &[pin, signal] : signals)
      nodeNames[signal.node] = path + "." + pin;
    instances.push_back({path, chip, !d.builtin.empty(), signals});
    loading.pop_back();
    return signals;
  }
  void schedule() {
    std::map<int, int> producers;
    for (int i = 0; i < int(devices.size()); ++i)
      for (auto &n : devices[i].outputs)
        for (int bit : devices[i].pins.at(n).bits) {
          int id = find(bit);
          if (producers.contains(id) && producers[id] != i)
            throw Error("Multiple drivers on one wire");
          producers[id] = i;
        }
    std::vector<std::set<int>> edges(devices.size());
    std::vector<int> indegree(devices.size());
    for (int i = 0; i < int(devices.size()); ++i) {
      auto &d = devices[i];
      for (auto &n : d.inputs) {
        if (d.clocked.contains(n))
          continue;
        for (int bit : d.pins.at(n).bits) {
          auto p = producers.find(find(bit));
          if (p != producers.end() && edges[p->second].insert(i).second)
            ++indegree[i];
        }
      }
    }
    std::deque<int> ready;
    for (int i = 0; i < int(devices.size()); ++i)
      if (indegree[i] == 0)
        ready.push_back(i);
    while (!ready.empty()) {
      int i = ready.front();
      ready.pop_front();
      order.push_back(i);
      for (int n : edges[i])
        if (--indegree[n] == 0)
          ready.push_back(n);
    }
    if (order.size() != devices.size())
      throw Error("This chip has a circle in its parts connections");
    // Canonicalize all pin references once, keeping future snapshot copies
    // independent.
    for (auto &[n, s] : rootPins)
      for (auto &bit : s.bits)
        bit = find(bit);
    for (auto &d : devices)
      for (auto &[n, s] : d.pins)
        for (auto &bit : s.bits)
          bit = find(bit);
    writeNode(1, 65535);
    writeNode(2, 1);
  }
  Word input(const Device &d, const std::string &pin) const {
    auto i = d.pins.find(d.roles.contains(pin) ? d.roles.at(pin) : pin);
    if (i == d.pins.end())
      throw Error("Built-in " + d.kind + " requires pin " + pin);
    return read(i->second);
  }
  void output(Device &d, const std::string &pin, Word value) {
    auto i = d.pins.find(d.roles.contains(pin) ? d.roles.at(pin) : pin);
    if (i == d.pins.end())
      throw Error("Built-in " + d.kind + " requires pin " + pin);
    write(i->second, value);
  }
  void compute(Device &d, bool force = false) {
    std::vector<Word> inputs;
    for (auto &n : d.inputs)
      inputs.push_back(read(d.pins.at(n)));
    if (!force && !d.dirty && inputs == d.lastInputs)
      return;
    d.lastInputs = inputs;
    d.dirty = false;
    auto in = [&](const std::string &n) { return input(d, n); };
    auto out = [&](Word v) { output(d, "out", v); };
    auto k = d.kind;
    if (reg(k))
      return;
    if (!d.memory.empty()) {
      out(d.memory.at(in("address")));
      return;
    }
    if (k == "Keyboard") {
      out(d.state);
      return;
    }
    if (k == "Nand")
      out(Word(1 - (in("a") & in("b"))));
    else if (k == "Not")
      out(Word(1 - in("in")));
    else if (k == "Not16")
      out(Word(~in("in")));
    else if (k == "And")
      out(Word(in("a") & in("b")));
    else if (k == "Or")
      out(Word(in("a") | in("b")));
    else if (k == "Xor")
      out(Word(in("a") ^ in("b")));
    else if (k == "Mux")
      out(in("sel") == 0 ? in("a") : in("b"));
    else if (k == "Mux4Way16" || k == "Mux8Way16") {
      int count = k == "Mux4Way16" ? 4 : 8;
      auto sel = in("sel");
      out(sel < count ? in(std::string(1, char('a' + sel))) : 0);
    } else if (k == "DMux") {
      output(d, "a", in("sel") == 0 ? in("in") : 0);
      output(d, "b", in("sel") == 0 ? 0 : in("in"));
    } else if (k == "DMux4Way" || k == "DMux8Way") {
      int count = k == "DMux4Way" ? 4 : 8;
      for (int n = 0; n < count; ++n)
        output(d, std::string(1, char('a' + n)), n == in("sel") ? in("in") : 0);
    } else if (k == "Or8Way")
      out(in("in") != 0 ? 1 : 0);
    else if (k == "HalfAdder") {
      output(d, "sum", Word(in("a") ^ in("b")));
      output(d, "carry", Word(in("a") & in("b")));
    } else if (k == "FullAdder") {
      int sum = signedWord(Word(std::uint32_t(in("a")) + in("b") + in("c")));
      output(d, "sum", Word(sum % 2));
      output(d, "carry", Word(sum / 2));
    } else if (k == "Add16")
      out(Word(std::uint32_t(in("a")) + in("b")));
    else if (k == "Inc16")
      out(Word(std::uint32_t(in("in")) + 1));
    else if (k == "ALU") {
      Word x = in("x"), y = in("y");
      if (in("zx") == 1)
        x = 0;
      if (in("nx") == 1)
        x = Word(~x);
      if (in("zy") == 1)
        y = 0;
      if (in("ny") == 1)
        y = Word(~y);
      Word result = in("f") == 1 ? Word(std::uint32_t(x) + y) : Word(x & y);
      if (in("no") == 1)
        result = Word(~result);
      out(result);
      output(d, "zr", result == 0 ? 1 : 0);
      output(d, "ng", result & 0x8000 ? 1 : 0);
    }
  }
  void eval() {
    for (int i : order)
      compute(devices[i]);
  }
  Device &component(const std::string &n) {
    for (auto &d : devices)
      if (d.path == n || d.chip == n)
        return d;
    throw Error("No such built-in chip used: " + n);
  }
  const Device &component(const std::string &n) const {
    for (auto &d : devices)
      if (d.path == n || d.chip == n)
        return d;
    throw Error("No such built-in chip used: " + n);
  }
};
Hardware::Hardware() : impl_(std::make_unique<Impl>()) {}
Hardware::~Hardware() = default;
Hardware::Hardware(const Hardware &h)
    : impl_(std::make_unique<Impl>(*h.impl_)) {}
Hardware &Hardware::operator=(const Hardware &h) {
  if (this != &h)
    impl_ = std::make_unique<Impl>(*h.impl_);
  return *this;
}
Hardware::Hardware(Hardware &&) noexcept = default;
Hardware &Hardware::operator=(Hardware &&) noexcept = default;
void Hardware::load(const std::string &chip, const HdlResolver &resolver,
                    const std::function<bool()> &cancelled) {
  auto next = std::make_unique<Impl>();
  next->name = chip;
  next->net();
  next->net();
  next->net();
  next->nodes.resize(3);
  next->rootPins = next->instantiate(chip, chip, resolver, cancelled);
  next->schedule();
  if (cancelled && cancelled())
    throw Error("Cancelled");
  impl_ = std::move(next);
}
void Hardware::loadFile(const std::filesystem::path &file) {
  auto base = file.parent_path();
  load(file.stem().string(),
       [base](const std::string &n) -> std::optional<std::string> {
         auto p = base / (n + ".hdl");
         if (std::filesystem::exists(p))
           return readFile(p);
         return std::nullopt;
       });
}
bool Hardware::loaded() const { return !impl_->name.empty(); }
std::string Hardware::name() const { return impl_->name; }
void Hardware::eval() {
  if (!loaded())
    throw Error("No chip loaded");
  impl_->eval();
}
void Hardware::tick() {
  if (!loaded())
    throw Error("No chip loaded");
  if (impl_->up)
    throw Error("Illegal command since clock is already up");
  impl_->writeNode(2, 0);
  impl_->eval();
  // Inputs settle before sampling. Sequential outputs remain unchanged until
  // tock.
  for (int i : impl_->order) {
    auto &d = impl_->devices[i];
    impl_->compute(d, true);
    auto in = [&](const std::string &n) { return impl_->input(d, n); };
    if (d.kind == "DFF")
      d.state = in("in");
    else if (d.kind == "PC") {
      if (in("reset") == 1)
        d.state = 0;
      else if (in("load") == 1)
        d.state = in("in");
      else if (in("inc") == 1)
        d.state = Word(std::uint32_t(d.state) + 1);
    } else if (reg(d.kind)) {
      if (in("load") == 1)
        d.state = in("in");
    } else if (!d.memory.empty() && d.kind != "ROM32K" && in("load") == 1)
      d.memory.at(in("address")) = in("in");
  }
  impl_->up = true;
}
void Hardware::tock() {
  if (!loaded())
    throw Error("No chip loaded");
  if (!impl_->up)
    throw Error("Illegal command since clock is already down");
  impl_->writeNode(2, 1);
  for (int i : impl_->order) {
    auto &d = impl_->devices[i];
    if (reg(d.kind))
      impl_->output(d, "out", d.state);
    impl_->compute(d, true);
  }
  impl_->up = false;
  ++impl_->time;
}
bool Hardware::clockUp() const { return impl_->up; }
std::uint64_t Hardware::time() const { return impl_->time; }
int Hardware::get(const std::string &n) const {
  auto p = impl_->rootPins.find(n);
  if (p != impl_->rootPins.end()) {
    return signedWord(impl_->read(p->second));
  }
  auto b = n.rfind('[');
  if (b == n.npos || !n.ends_with("]"))
    throw Error("Unknown variable " + n);
  auto &d = impl_->component(n.substr(0, b));
  int index = parseIndex(n.substr(b + 1, n.size() - b - 2));
  if (!d.memory.empty()) {
    if (std::size_t(index) >= d.memory.size())
      throw Error("Illegal index");
    return signedWord(d.memory[index]);
  }
  if (index == 0 && (inspectableRegister(d.kind) || d.kind == "Keyboard"))
    return signedWord(d.state);
  if (d.kind == "ALU")
    throw Error("ALU cannot be used as a variable: " + n);
  throw Error("No such built-in chip used: " + d.chip);
}
std::string Hardware::getText(const std::string &n) const {
  return n == "time" ? std::to_string(impl_->time) + (impl_->up ? "+" : " ")
                     : std::to_string(get(n));
}
void Hardware::set(const std::string &n, int v) {
  if (v < -32768 || v > 32767)
    throw Error("Value outside signed 16-bit range");
  auto p = impl_->rootPins.find(n);
  if (p != impl_->rootPins.end()) {
    if (p->second.direction != "input")
      throw Error("Read Only variable: " + n);
    if (v >= 0 && std::uint32_t(v) > ((1u << p->second.bits.size()) - 1u))
      throw Error("Value doesn't fit in the pin's width: " + n);
    impl_->write(p->second, Word(v));
    return;
  }
  auto b = n.rfind('[');
  if (b == n.npos || !n.ends_with("]"))
    throw Error("Unknown or read-only variable " + n);
  auto &d = impl_->component(n.substr(0, b));
  int index = parseIndex(n.substr(b + 1, n.size() - b - 2));
  if (d.kind == "Keyboard")
    throw Error("Keyboard is read only: " + n);
  if (d.kind == "ALU")
    throw Error("ALU cannot be used as a variable: " + n);
  if (!d.memory.empty()) {
    if (std::size_t(index) >= d.memory.size())
      throw Error("Illegal index");
    d.memory[index] = Word(v);
  } else if (index == 0 && inspectableRegister(d.kind)) {
    d.state = Word(v);
    impl_->output(d, "out", Word(v));
  } else
    throw Error("Component has no writable memory: " + n);
  d.dirty = true;
  impl_->eval();
}
void Hardware::loadRom(const std::string &component,
                       const std::vector<Word> &program) {
  auto &d = impl_->component(component);
  if (d.kind != "ROM32K")
    throw Error("Only ROM32K supports program loading");
  if (program.size() > d.memory.size())
    throw Error("Program too large");
  std::fill(d.memory.begin(), d.memory.end(), Word(0));
  std::copy(program.begin(), program.end(), d.memory.begin());
  d.dirty = true;
  impl_->eval();
}
void Hardware::keyboard(Word v) {
  bool changed = false;
  for (auto &d : impl_->devices)
    if (d.kind == "Keyboard" && d.state != v) {
      d.state = v;
      impl_->output(d, "out", v);
      changed = true;
    }
  if (changed)
    impl_->eval();
}
std::vector<Word> Hardware::screen() const {
  for (auto &d : impl_->devices)
    if (d.kind == "Screen")
      return d.memory;
  return {};
}
std::vector<HdlPin> Hardware::pins() const {
  std::vector<HdlPin> out;
  for (auto &[n, p] : impl_->rootPins)
    out.push_back({n, p.direction, int(p.bits.size()), impl_->read(p)});
  return out;
}
std::vector<HdlComponent> Hardware::components() const {
  std::vector<HdlComponent> out;
  for (auto &d : impl_->devices) {
    HdlComponent c{d.path, d.chip, d.kind, d.memory.size(), {}};
    for (auto &[n, p] : d.pins)
      c.pins.push_back(
          {n, p.direction, int(p.bits.size()), impl_->read(p)});
    out.push_back(std::move(c));
  }
  return out;
}
std::vector<std::string> Hardware::builtins() {
  std::vector<std::string> out;
  for (auto [n, s] : builtinHdl)
    out.emplace_back(n);
  return out;
}
std::vector<HdlInstance> Hardware::hierarchy() const {
  std::vector<HdlInstance> result;
  for (const auto &instance : impl_->instances) {
    HdlInstance item{instance.path, instance.chip, instance.builtin, {}};
    for (const auto &[name, signal] : instance.pins)
      item.pins.push_back({name, signal.direction, int(signal.bits.size()), impl_->read(signal)});
    result.push_back(std::move(item));
  }
  std::sort(result.begin(),result.end(),[](const auto& a,const auto& b){return a.path<b.path;});
  return result;
}
std::vector<HdlWire> Hardware::wires() const {
  std::vector<HdlWire> result;
  for (std::size_t source=0;source<impl_->nodes.size();++source)
    for (const auto &wire : impl_->nodes[source].listeners) {
      const Word value=impl_->nodes[source].value;
      result.push_back({impl_->nodeNames.at(int(source)),impl_->nodeNames.at(wire.target),
                        wire.sourceLo,wire.targetLo,wire.width,
                        wire.sourceLo<0?value:Word((value>>wire.sourceLo)&((1u<<wire.width)-1u))});
    }
  return result;
}
} // namespace nand
