// SPDX-License-Identifier: GPL-3.0-or-later
#include "core.hpp"
#include <iostream>
#include <algorithm>
#ifdef _WIN32
#include <windows.h>
#endif
namespace fs=std::filesystem;
std::string nativeNewlines(std::string s){
#ifdef _WIN32
    std::string r;for(char c:s){if(c=='\n')r+='\r';r+=c;}return r;
#else
    return s;
#endif
}
int run(const std::vector<fs::path>& argv){
    auto tool=argv[0].stem().string();std::size_t first=1;
    if(tool=="nand"){if(argv.size()<2){std::cout<<"Usage: nand <Assembler|JackCompiler|CPUEmulator|VMEmulator|TextComparer> [arguments]\n";return 0;}tool=argv[first++].string();}
    std::vector<fs::path> args(argv.begin()+first,argv.end());
    if(!args.empty()&&(args[0]=="/?"||args[0]=="-h"||args[0]=="--help")){std::cout<<"Usage: "<<tool<<" "<<(tool=="TextComparer"?"FILE1 FILE2":tool=="JackCompiler"?"[FILE.jack | DIRECTORY]":tool=="Assembler"?"FILE.asm":"SCRIPT.tst")<<"\n";return 0;}
    try{
        if(tool=="TextComparer"){if(args.size()!=2)throw nand::Error("Usage: TextComparer FILE1 FILE2");auto r=nand::compareText(nand::readFile(args[0]),nand::readFile(args[1]));std::cout<<r.message;return r.equal?0:-1;}
        if(tool=="Assembler"){if(args.size()!=1)throw nand::Error("Open NandStudio for interactive assembly; batch usage: Assembler FILE.asm");auto path=fs::absolute(args[0]);if(!path.has_extension())path+=".asm";std::cout<<"Assembling \""<<path.string()<<"\"\n";auto result=nand::assemble(nand::readFile(path));path.replace_extension(".hack");nand::writeFile(path,nativeNewlines(nand::machineText(result.words)));return 0;}
        if(tool=="JackCompiler"){
            if(args.size()>1)throw nand::Error("Usage: JackCompiler [FILE.jack | DIRECTORY]");auto target=fs::absolute(args.empty()?fs::current_path():args[0]);std::cout<<"Compiling \""<<target.string()<<"\"\n";
            std::vector<fs::path> files;if(fs::is_directory(target)){for(auto& e:fs::directory_iterator(target))if(e.path().extension()==".jack")files.push_back(e.path());}else files.push_back(target);std::sort(files.begin(),files.end());
            for(auto& path:files){try{auto output=nand::compileJack(nand::readFile(path));auto dest=path;dest.replace_extension(".vm");nand::writeFile(dest,nativeNewlines(output));}catch(nand::Error& e){e.file=path.string();throw;}}return 0;
        }
        if(tool=="CPUEmulator"||tool=="VMEmulator"||tool=="HardwareSimulator"){if(args.size()!=1)throw nand::Error("Open NandStudio for interactive execution; batch usage: "+tool+" SCRIPT.tst");auto r=nand::runScript(fs::absolute(args[0]),tool=="HardwareSimulator"?nand::ScriptTool::Hardware:tool=="VMEmulator"?nand::ScriptTool::Vm:nand::ScriptTool::Cpu);(r.passed?std::cout:std::cerr)<<r.message<<"\n";return r.passed?0:-1;}
        throw nand::Error("Unknown tool "+tool);
    }catch(const nand::Error& e){if(tool=="Assembler")std::cerr<<"In line "<<e.line<<", ";else if(tool!="CPUEmulator"&&tool!="VMEmulator"&&tool!="HardwareSimulator"&&!e.file.empty())std::cerr<<e.file<<":"<<e.line<<":"<<e.column<<": ";std::cerr<<e.what()<<"\n";return -1;}
    catch(const std::exception& e){std::cerr<<e.what()<<"\n";return -1;}
}
#ifdef _WIN32
int wmain(int argc,wchar_t** argv){std::vector<fs::path> paths;for(int i=0;i<argc;++i)paths.emplace_back(argv[i]);return run(paths);}
#else
int main(int argc,char** argv){std::vector<fs::path> paths;for(int i=0;i<argc;++i)paths.emplace_back(argv[i]);return run(paths);}
#endif
