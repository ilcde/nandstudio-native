// SPDX-License-Identifier: GPL-3.0-or-later
// Independently authored fixtures; never replace the bundled student starters.
#include "hdl.hpp"
#include <iostream>
#include <map>
#include <random>
int main() {
  int checks = 0;
  auto check = [&](bool b) {
    if (!b)
      throw std::runtime_error("Hardware check " + std::to_string(checks + 1) +
                               " failed");
    ++checks;
  };
  try {
    nand::Hardware h;
    std::map<std::string, std::string> files;
    auto load = [&](const std::string &n) {
      h.load(n, [&](const std::string &name) -> std::optional<std::string> {
        auto i = files.find(name);
        if (i == files.end())
          return std::nullopt;
        return i->second;
      });
    };
    check(nand::Hardware::builtins().size() == 35);
    for (auto &name : nand::Hardware::builtins()) {
      load(name);
      h.eval();
      check(h.loaded());
    }
    load("Nand");
    for (int a = 0; a < 2; ++a)
      for (int b = 0; b < 2; ++b) {
        h.set("a", a);
        h.set("b", b);
        h.eval();
        check(h.get("out") == 1 - (a & b));
      }
    std::mt19937 rng(2026);
    load("Not");
    h.set("in", -1); h.eval();
    check(h.get("in") == -1 && h.get("out") == 2);
    files["SignedWire"] = "CHIP SignedWire { IN in; OUT out; PARTS: "
                          "Not(in=in,out=n); Not(in=n,out=out); }";
    load("SignedWire"); h.set("in", -32768); h.eval();
    check(h.get("in") == -32768 && h.get("n") == -32767 && h.get("out") == -32768);
    files["SlicedWire"] = "CHIP SlicedWire { IN in; OUT out[2]; PARTS: "
                         "Not(in=in[0],out[0]=out[0],out[0]=out[1]); }";
    load("SlicedWire"); h.set("in", -2); h.eval();
    check(h.get("in") == -2 && h.get("out") == 3);
    h.set("in", -1); h.eval(); check(h.get("out") == 0);
    load("HalfAdder"); h.set("a", -1); h.set("b", 1); h.eval();
    check(h.get("sum") == -2 && h.get("carry") == 1);
    load("FullAdder"); h.set("a", -1); h.set("b", -1); h.set("c", -1); h.eval();
    check(h.get("sum") == -1 && h.get("carry") == -1);
    load("DMux"); h.set("in", -2); h.set("sel", -1); h.eval();
    check(h.get("a") == 0 && h.get("b") == -2);
    load("DFF"); h.set("in", -32768); h.eval();
    check(h.get("out") == 0); h.tick(); check(h.get("out") == 0);
    h.tock(); check(h.get("out") == -32768);
    load("ALU");
    for (int i = 0; i < 2000; ++i) {
      nand::Word x = nand::Word(rng()), y = nand::Word(rng());
      int flags = int(rng() % 64);
      h.set("x", nand::signedWord(x));
      h.set("y", nand::signedWord(y));
      for (int n = 0; n < 6; ++n)
        h.set(std::vector<std::string>{"zx", "nx", "zy", "ny", "f", "no"}[n],
              (flags >> n) & 1);
      if (flags & 1)
        x = 0;
      if (flags & 2)
        x = nand::Word(~x);
      if (flags & 4)
        y = 0;
      if (flags & 8)
        y = nand::Word(~y);
      nand::Word result =
          flags & 16 ? nand::Word(std::uint32_t(x) + y) : nand::Word(x & y);
      if (flags & 32)
        result = nand::Word(~result);
      h.eval();
      check(h.get("out") == nand::signedWord(result) &&
            h.get("zr") == int(result == 0) &&
            h.get("ng") == int((result & 32768) != 0));
    }
    load("PC");
    h.set("in", 32767);
    h.set("load", 1);
    h.tick();
    check(h.get("out") == 0 && h.getText("time") == "0+");
    h.tock();
    check(h.get("out") == 32767);
    h.set("load", 0);
    h.set("inc", 1);
    h.tick();
    h.tock();
    check(h.get("out") == -32768);
    h.set("load", 1);
    h.set("reset", 1);
    h.tick();
    h.tock();
    check(h.get("out") == 0 && h.getText("time") == "3 ");
    load("RAM8");
    h.set("in", 123);
    h.set("load", 1);
    h.set("address", 3);
    h.tick();
    check(h.get("RAM8[3]") == 123 && h.get("out") == 0);
    h.tock();
    check(h.get("out") == 123);
    h.set("address", 4);
    h.eval();
    check(h.get("out") == 0);
    h.set("address", 3);
    h.eval();
    check(h.get("out") == 123);
    files["Invert"] =
        "CHIP Invert { IN in; OUT out; PARTS: Nand(a=in,b=in,out=out); }";
    files["Chain"] = "CHIP Chain { IN in; OUT out; PARTS: Invert(in=in,out=a); "
                     "Invert(in=a,out=out); }";
    load("Chain");
    h.set("in", 1);
    h.eval();
    check(h.get("out") == 1 && h.get("a") == 0);
    auto copy = h;
    copy.set("in", 0);
    copy.eval();
    check(copy.get("out") == 0 && h.get("out") == 1);
    files["Delay"] = "CHIP Delay { IN in; OUT first,second; PARTS: "
                     "DFF(in=in,out=a,out=first); DFF(in=a,out=second); }";
    load("Delay");
    h.set("in", 1);
    h.tick();
    check(h.get("first") == 0 && h.get("second") == 0);
    h.tock();
    check(h.get("first") == 1 && h.get("second") == 0);
    h.tick();
    h.tock();
    check(h.get("second") == 1);
    files["Feedback"] = "CHIP Feedback { OUT out; PARTS: Not(in=q,out=n); "
                        "DFF(in=n,out=q,out=out); }";
    load("Feedback");
    h.eval();
    h.tick();
    h.tock();
    check(h.get("out") == 1);
    h.tick();
    h.tock();
    check(h.get("out") == 0);
    files["Bus"] = "CHIP Bus { IN in[16]; OUT low[8],high[8]; PARTS: "
                   "And16(a=in,b=true,out[0..7]=low,out[8..15]=high); }";
    load("Bus");
    h.set("in", 0x1234);
    h.eval();
    check(h.get("low") == 0x34 && h.get("high") == 0x12);
    load("ROM32K");
    h.loadRom("ROM32K", {7, 65535});
    check(h.get("out") == 7);
    h.set("address", 1);
    h.eval();
    check(h.get("out") == -1);
    h.loadRom("ROM32K", {9});
    check(h.get("out") == 0);
    load("Screen");
    h.set("in", 1);
    h.set("load", 1);
    h.tick();
    h.tock();
    check(h.screen().size() == 8192 && h.screen()[0] == 1);
    load("Keyboard");
    h.keyboard(140);
    h.eval();
    check(h.get("out") == 140);
    const std::vector<std::string> bad = {
        "",
        "CHIP Bad { IN a[0]; PARTS: }",
        "CHIP Bad { PARTS: Bad(); }",
        "CHIP Bad { PARTS: Not(in=x,out=x); }",
        "CHIP Bad { PARTS: Not(in=q,out=r); }",
        "CHIP Bad { OUT out; PARTS: Not(in=true,out=a); Not(in=a[0],out=out); "
        "}",
        "CHIP Bad { OUT out; PARTS: Not(in=false,out=out); "
        "Not(in=true,out=out); }",
        "CHIP Bad { PARTS: /*"};
    for (auto &source : bad) {
      files["Bad"] = source;
      bool rejected = false;
      try {
        load("Bad");
      } catch (const nand::Error &) {
        rejected = true;
      }
      check(rejected && h.name() == "Keyboard" && h.get("out") == 140);
    }
    bool rejected = false;
    try {
      h.tock();
    } catch (const nand::Error &) {
      rejected = true;
    }
    check(rejected);
    rejected = false;
    try {
      h.load(
          "Nand",
          [](const std::string &) -> std::optional<std::string> {
            return std::nullopt;
          },
          [] { return true; });
    } catch (const nand::Error &) {
      rejected = true;
    }
    check(rejected && h.name() == "Keyboard");
    std::string incomplete =
        "CHIP Bad { IN in; OUT out; PARTS: Not(in=in,out=out); }";
    for (std::size_t n = 0; n < incomplete.size(); ++n) {
      files["Bad"] = incomplete.substr(0, n);
      rejected = false;
      try {
        load("Bad");
      } catch (const nand::Error &) {
        rejected = true;
      }
      check(rejected && h.name() == "Keyboard");
    }
    rejected = false;
    try {
      h.set("Keyboard[]", 65);
    } catch (const nand::Error &) {
      rejected = true;
    }
    check(rejected && h.get("out") == 140);
    std::cout << checks << " hardware checks passed\n";
  } catch (const std::exception &e) {
    std::cerr << e.what() << "\n";
    return 1;
  }
  return 0;
}
