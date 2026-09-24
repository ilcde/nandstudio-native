// SPDX-License-Identifier: GPL-3.0-or-later
#include "core.hpp"
#include <sstream>
#include <set>
namespace nand {
void Vm::load(const std::map<std::string,std::string>& files){
    Vm next;int staticNext=16;
    for(const auto& [file,text]:files){std::string scope=file;int n=0,maxStatic=-1;
        for(auto line:lines(text)){++n;auto comment=line.find("//");if(comment!=line.npos)line.resize(comment);std::istringstream in(line);VmInstruction i;i.file=file;i.line=n;i.scope=scope;if(!(in>>i.op))continue;
            const std::set<std::string> unary={"add","sub","neg","eq","gt","lt","and","or","not","return"};
            if(i.op=="push"||i.op=="pop"||i.op=="function"||i.op=="call"){
                if(!(in>>i.arg>>i.index)||i.index<0||i.index>32767)throw Error("Invalid VM operands",n,1,file);
                if(i.op=="push"||i.op=="pop"){
                    const std::set<std::string> segs={"constant","local","argument","this","that","pointer","temp","static"};
                    if(!segs.contains(i.arg)||(i.op=="pop"&&i.arg=="constant")||(i.arg=="pointer"&&i.index>1)||(i.arg=="temp"&&i.index>7))throw Error("Invalid VM segment",n,1,file);
                    if(i.arg=="static")maxStatic=std::max(maxStatic,i.index);
                }else if(i.op=="function"){scope=i.arg;i.scope=scope;if(line.starts_with("function ")){if(next.functions.contains(scope))throw Error("Duplicate function",n,1,file);next.functions[scope]=next.code.size();}}
            }else if(i.op=="label"||i.op=="goto"||i.op=="if-goto"){
                if(!(in>>i.arg))throw Error("Label expected",n,1,file);
                // Preserve the legacy symbol pass: indented declarations are not registered.
                if(i.op=="label"&&line.starts_with("label ")){auto key=scope+"$"+i.arg;next.labels[key]=next.code.size()+1;}
            }else if(!unary.contains(i.op))throw Error("Unknown VM command "+i.op,n,1,file);
            std::string extra;if(in>>extra)throw Error("Unexpected VM operand",n,1,file);
            next.code.push_back(std::move(i));if(next.code.size()>32767)throw Error("VM program too large");
        }
        next.statics[file]=staticNext;staticNext+=maxStatic+1;if(staticNext>256)throw Error("Static segment exceeds available memory");
    }
    for(const auto& i:next.code)if((i.op=="goto"||i.op=="if-goto")&&!next.labels.contains(i.scope+"$"+i.arg))throw Error(i.file+".vm: in line "+std::to_string(i.line)+": Unknown label - "+i.scope+"$"+i.arg,i.line,1,i.file);
    if(next.functions.contains("Sys.init")){next.ram[0]=256;next.pc=next.functions.at("Sys.init");}
    *this=std::move(next);
}
Word Vm::pop(){if(ram[0]==0||ram[0]>32768)throw Error("Stack underflow or invalid SP");return ram[--ram[0]];}
void Vm::push(Word v){if(ram[0]>=32768)throw Error("Stack overflow or invalid SP");ram[ram[0]++]=v;}
std::size_t Vm::address(const VmInstruction& i)const{
    int base=0;if(i.arg=="local")base=ram[1];else if(i.arg=="argument")base=ram[2];else if(i.arg=="this")base=ram[3];else if(i.arg=="that")base=ram[4];else if(i.arg=="pointer")base=3;else if(i.arg=="temp")base=5;else if(i.arg=="static")base=statics.at(i.file);else throw Error("Invalid segment");
    auto a=base+i.index;if(a<0||a>=32768)throw Error("Illegal memory address");return static_cast<std::size_t>(a);
}
void Vm::step(){
    if(pc>=code.size())throw Error("VM program counter outside program");const auto i=code[pc];++pc;
    while(pc<code.size()&&code[pc].op=="label")++pc;
    try{
        if(i.op=="push")push(i.arg=="constant"?Word(i.index):ram[address(i)]);
        else if(i.op=="pop"){auto a=address(i);auto v=pop();ram[a]=v;}
        else if(i.op=="label"){}
        else if(i.op=="goto")pc=labels.at(i.scope+"$"+i.arg);
        else if(i.op=="if-goto"){if(pop()!=0)pc=labels.at(i.scope+"$"+i.arg);}
        else if(i.op=="function"){for(int n=0;n<i.index;++n)push(0);}
        else if(i.op=="call"){
            if(!functions.contains(i.arg))throw Error("Unresolved function "+i.arg+"; native built-in fallback is not implemented");
            if(ram[0]<i.index||ram[0]>32762)throw Error("Invalid call frame");
            push(Word(pc));for(int n=1;n<=4;++n)push(ram[n]);ram[2]=Word(ram[0]-5-i.index);ram[1]=ram[0];pc=functions.at(i.arg);callStack.push_back(i.arg);
        }else if(i.op=="return"){
            auto frame=ram[1];if(frame<5||frame>=32768||ram[2]>=32768)throw Error("Invalid return frame");
            auto ret=ram[frame-5];ram[13]=frame;ram[14]=ret;auto value=pop();ram[ram[2]]=value;ram[0]=Word(ram[2]+1);
            for(int n=4;n>=1;--n)ram[n]=ram[frame-(5-n)];pc=ret;if(!callStack.empty())callStack.pop_back();
        }else if(i.op=="neg")push(Word(0u-pop()));else if(i.op=="not")push(Word(~pop()));
        else{auto y=pop(),x=pop();Word v=0;
            if(i.op=="add")v=Word(std::uint32_t(x)+y);else if(i.op=="sub")v=Word(std::uint32_t(x)-y);else if(i.op=="and")v=x&y;else if(i.op=="or")v=x|y;
            else if(i.op=="eq")v=x==y?65535:0;else if(i.op=="gt")v=signedWord(x)>signedWord(y)?65535:0;else if(i.op=="lt")v=signedWord(x)<signedWord(y)?65535:0;
            push(v);
        }++time;
    }catch(Error& e){e.file=i.file;e.line=i.line;throw;}
}
int Vm::get(std::string_view name)const{
    if(name=="PC")return int(pc);if(name=="time")return int(time);static const std::map<std::string,int> names={{"SP",0},{"LCL",1},{"ARG",2},{"THIS",3},{"THAT",4}};
    if(names.contains(std::string(name)))return signedWord(ram[names.at(std::string(name))]);
    static const std::map<std::string,int> pointers={{"sp",0},{"local",1},{"argument",2},{"this",3},{"that",4}};
    if(pointers.contains(std::string(name)))return signedWord(ram[pointers.at(std::string(name))]);
    auto b=name.find('[');if(b!=name.npos&&name.ends_with("]")){
        auto index=std::string(name.substr(b+1,name.size()-b-2));std::size_t used=0;int n;try{n=std::stoi(index,&used);}catch(...){throw Error("Invalid memory index");}if(used!=index.size()||n<0)throw Error("Invalid memory index");
        auto seg=std::string(name.substr(0,b));std::size_t a;if(seg=="RAM")a=static_cast<std::size_t>(n);else {VmInstruction i;i.arg=seg;i.index=n;i.file=pc<code.size()?code[pc].file:statics.begin()->first;a=address(i);}if(a>=ram.size())throw Error("Invalid memory index");return signedWord(ram[a]);
    }throw Error("Unknown variable "+std::string(name));
}
void Vm::set(std::string_view name,int v){
    static const std::map<std::string,int> pointers={{"sp",0},{"local",1},{"argument",2},{"this",3},{"that",4}};
    if(pointers.contains(std::string(name))){ram[pointers.at(std::string(name))]=Word(v);return;}
    static const std::map<std::string,int> names={{"SP",0},{"LCL",1},{"ARG",2},{"THIS",3},{"THAT",4}};
    if(names.contains(std::string(name))){ram[names.at(std::string(name))]=Word(v);return;}
    if(name=="PC"){if(v<0||std::size_t(v)>=code.size())throw Error("Invalid PC");pc=std::size_t(v);return;}
    auto b=name.find('[');if(b!=name.npos&&name.ends_with("]")){get(name);auto n=std::stoi(std::string(name.substr(b+1)));auto seg=std::string(name.substr(0,b));VmInstruction i;i.arg=seg;i.index=n;i.file=pc<code.size()?code[pc].file:statics.begin()->first;auto a=seg=="RAM"?std::size_t(n):address(i);ram[a]=Word(v);return;}
    throw Error("Unknown or read-only variable "+std::string(name));
}
}
