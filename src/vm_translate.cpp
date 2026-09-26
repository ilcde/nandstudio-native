// SPDX-License-Identifier: GPL-3.0-or-later
#include "core.hpp"
#include <set>
#include <cctype>
namespace nand {
std::string translateVm(const std::map<std::string,std::string>& files,bool bootstrap){
    std::map<std::string,std::string> normalized;
    std::string prefix="__NAND$";
    for(;;){bool collision=false;for(const auto& [name,text]:files)if(name.find(prefix)!=name.npos||text.find(prefix)!=text.npos)collision=true;if(!collision)break;prefix+='$';}
    // The translator accepts ordinary leading whitespace without changing the
    // legacy emulator's separately documented declaration-pass behavior.
    for(const auto& [name,text]:files)for(auto line:lines(text)){
        for(auto& c:line)if(c=='\t')c=' ';
        const auto first=line.find_first_not_of(" \t\r");
        normalized[name]+=(first==line.npos?std::string():line.substr(first))+"\n";
    }
    Vm parsed;parsed.load(normalized);
    auto symbol=[](const std::string& s){if(s.empty()||std::isdigit(static_cast<unsigned char>(s[0])))return false;for(unsigned char c:s)if(!(std::isalnum(c)||c=='_'||c=='.'||c=='$'||c==':'))return false;return true;};
    std::set<std::string> declarations;
    const std::set<std::string> reserved={"SP","LCL","ARG","THIS","THAT","SCREEN","KBD","R0","R1","R2","R3","R4","R5","R6","R7","R8","R9","R10","R11","R12","R13","R14","R15"};
    for(const auto& i:parsed.code){
        auto name=i.op=="function"||i.op=="call"?i.arg:i.scope+"$"+i.arg;
        if(i.op=="function"||i.op=="call"||i.op=="label"||i.op=="goto"||i.op=="if-goto"){
            if(!symbol(name)||reserved.contains(name))throw Error("Invalid or reserved translation symbol: "+name,i.line,1,i.file);
            if((i.op=="function"||i.op=="label")&&!declarations.insert(name).second)throw Error("Duplicate translation symbol: "+name,i.line,1,i.file);
            if(i.op=="call"&&!parsed.functions.contains(i.arg))throw Error("Translation requires implementation of "+i.arg,i.line,1,i.file);
        }
    }
    if(bootstrap&&!parsed.functions.contains("Sys.init"))throw Error("Bootstrap requires Sys.init");
    std::string out;unsigned serial=0;
    auto emit=[&](const std::string& s){out+=s;if(out.size()>8*1024*1024)throw Error("Translation output resource limit exceeded");};
    auto push=[&]{emit("@SP\nA=M\nM=D\n@SP\nM=M+1\n");};
    auto pop=[&]{emit("@SP\nAM=M-1\nD=M\n");};
    auto call=[&](const std::string& name,int count){
        const auto ret=prefix+"RET"+std::to_string(serial++);
        emit("@"+ret+"\nD=A\n");push();
        for(const auto& reg:{"LCL","ARG","THIS","THAT"}){emit(std::string("@")+reg+"\nD=M\n");push();}
        emit("@SP\nD=M\n@5\nD=D-A\n@"+std::to_string(count)+"\nD=D-A\n@ARG\nM=D\n@SP\nD=M\n@LCL\nM=D\n@"+name+"\n0;JMP\n("+ret+")\n");
    };
    if(bootstrap){emit("@256\nD=A\n@SP\nM=D\n");call("Sys.init",0);emit("@"+prefix+"HALT\n0;JMP\n");}
    for(const auto& i:parsed.code){
        const auto& op=i.op;const auto n=std::to_string(i.index);
        if(op=="push"||op=="pop"){
            if(i.arg=="constant")emit("@"+n+"\nD=A\n");
            else{
                const bool indirect=i.arg=="local"||i.arg=="argument"||i.arg=="this"||i.arg=="that";
                if(indirect){const auto reg=i.arg=="local"?"LCL":i.arg=="argument"?"ARG":i.arg=="this"?"THIS":"THAT";emit(std::string("@")+reg+"\nD=M\n@"+n+"\nD=D+A\n");}
                else{const int address=(i.arg=="pointer"?3:i.arg=="temp"?5:parsed.statics.at(i.file))+i.index;emit("@"+std::to_string(address)+"\nD=A\n");}
                if(op=="push")emit("A=D\nD=M\n");else {emit("@R13\nM=D\n");pop();emit("@R13\nA=M\nM=D\n");}
            }
            if(op=="push")push();
        }else if(op=="label")emit("("+i.scope+"$"+i.arg+")\n");
        else if(op=="goto"||op=="if-goto"){if(op=="if-goto")pop();emit("@"+i.scope+"$"+i.arg+"\n"+(op=="goto"?"0;JMP\n":"D;JNE\n"));}
        else if(op=="function"){emit("("+i.arg+")\n");for(int k=0;k<i.index;++k){emit("D=0\n");push();}}
        else if(op=="call")call(i.arg,i.index);
        else if(op=="return"){
            emit("@LCL\nD=M\n@R13\nM=D\n@5\nA=D-A\nD=M\n@R14\nM=D\n");pop();
            emit("@ARG\nA=M\nM=D\n@ARG\nD=M+1\n@SP\nM=D\n");
            for(const auto& reg:{"THAT","THIS","ARG","LCL"})emit(std::string("@R13\nAM=M-1\nD=M\n@")+reg+"\nM=D\n");
            emit("@R14\nA=M\n0;JMP\n");
        }else if(op=="neg"||op=="not")emit(std::string("@SP\nA=M-1\nM=")+(op=="neg"?"-M\n":"!M\n"));
        else if(op=="eq"||op=="gt"||op=="lt"){
            const auto tag=prefix+"CMP"+std::to_string(serial++);
            pop();emit("@R13\nM=D\n@SP\nA=M-1\nD=M\n");
            // Opposite signs must be decided before subtracting to avoid overflow.
            if(op!="eq")emit("@"+tag+"NEG\nD;JLT\n@R13\nD=M\n@"+tag+(op=="gt"?"TRUE":"FALSE")+"\nD;JLT\n@"+tag+"SUB\n0;JMP\n("+tag+"NEG)\n@R13\nD=M\n@"+tag+(op=="lt"?"TRUE":"FALSE")+"\nD;JGE\n");
            emit("("+tag+"SUB)\n@SP\nA=M-1\nD=M\n@R13\nD=D-M\n@"+tag+"TRUE\nD;"+(op=="eq"?"JEQ":op=="gt"?"JGT":"JLT")+"\n("+tag+"FALSE)\n@SP\nA=M-1\nM=0\n@"+tag+"END\n0;JMP\n("+tag+"TRUE)\n@SP\nA=M-1\nM=-1\n("+tag+"END)\n");
        }else {pop();emit(std::string("@SP\nA=M-1\nM=")+(op=="add"?"D+M":op=="sub"?"M-D":op=="and"?"D&M":"D|M")+"\n");}
    }
    emit("("+prefix+"HALT)\n@"+prefix+"HALT\n0;JMP\n");
    if(assemble(out).words.size()>32768)throw Error("Translated program exceeds Hack ROM capacity");
    return out;
}
}
