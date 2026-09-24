// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include <array>
#include <cstdint>
#include <filesystem>
#include <map>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>
#include <functional>
namespace nand {
using Word = std::uint16_t;
constexpr int signedWord(Word w) noexcept { return w < 32768 ? int(w) : int(w) - 65536; }
struct Error : std::runtime_error {
    std::string file;
    int line, column;
    Error(std::string message, int line = 1, int column = 1, std::string file = {});
};
std::string readFile(const std::filesystem::path& path);
void writeFile(const std::filesystem::path& path, std::string_view data);
std::vector<std::string> lines(std::string_view text);
struct Assembly { std::vector<Word> words; std::vector<int> sourceLines; std::map<std::string, Word> symbols; };
Assembly assemble(std::string_view source);
std::vector<Word> parseHack(std::string_view source);
std::string machineText(const std::vector<Word>& words);
struct Comparison { bool equal; int line; std::string message; };
Comparison compareText(std::string_view first, std::string_view second);
struct Cpu {
    std::array<Word, 32768> ram{}, rom{};
    Word a = 0, d = 0, pc = 0;
    std::uint64_t time = 0;
    Cpu();
    void load(const std::vector<Word>& program);
    void restart();
    void step();
    int get(std::string_view variable) const;
    void set(std::string_view variable, int value);
};
std::string compileJack(std::string_view source);
struct VmInstruction { std::string op, arg, file, scope; int index = 0, line = 1; };
struct Vm {
    std::array<Word, 32768> ram{};
    std::vector<VmInstruction> code;
    std::map<std::string, std::size_t> labels, functions;
    std::map<std::string, int> statics;
    std::vector<std::string> callStack;
    std::size_t pc = 0;
    std::uint64_t time = 0;
    void load(const std::map<std::string, std::string>& files);
    void step();
    int get(std::string_view variable) const;
    void set(std::string_view variable, int value);
private:
    Word pop(); void push(Word value); std::size_t address(const VmInstruction& i) const;
};
struct ScriptResult { bool passed = true; std::string message, output; std::uint64_t steps = 0; };
enum class ScriptTool { Cpu, Vm, Hardware };
ScriptResult runScript(const std::filesystem::path& path, ScriptTool tool, std::uint64_t budget = 10000000,
                       const std::function<bool()>& cancelled = {});
ScriptResult runScript(const std::filesystem::path& path, bool vm, std::uint64_t budget = 10000000,
                       const std::function<bool()>& cancelled = {});
}
