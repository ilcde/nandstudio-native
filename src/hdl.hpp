// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include "core.hpp"
#include <memory>
#include <optional>
namespace nand {
struct HdlPin {
  std::string name, direction;
  int width;
  Word value;
};
struct HdlComponent {
  std::string path, chip, implementation;
  std::size_t words;
  std::vector<HdlPin> pins;
};
// A resolver supplies project-local source, or nullopt to select the embedded
// built-in. URI-based storage can implement this without exposing a filesystem
// path to the engine.
using HdlResolver =
    std::function<std::optional<std::string>(const std::string &)>;
class Hardware {
public:
  Hardware();
  ~Hardware();
  Hardware(const Hardware &);
  Hardware &operator=(const Hardware &);
  Hardware(Hardware &&) noexcept;
  Hardware &operator=(Hardware &&) noexcept;
  void load(const std::string &chip, const HdlResolver &resolver,
            const std::function<bool()> &cancelled = {});
  void loadFile(const std::filesystem::path &file);
  bool loaded() const;
  std::string name() const;
  void eval();
  void tick();
  void tock();
  bool clockUp() const;
  std::uint64_t time() const;
  int get(const std::string &variable) const;
  std::string getText(const std::string &variable) const;
  void set(const std::string &variable, int value);
  void loadRom(const std::string &component, const std::vector<Word> &program);
  void keyboard(Word value);
  std::vector<Word> screen() const;
  std::vector<HdlPin> pins() const;
  std::vector<HdlComponent> components() const;
  static std::vector<std::string> builtins();

private:
  struct Impl;
  std::unique_ptr<Impl> impl_;
};
} // namespace nand
