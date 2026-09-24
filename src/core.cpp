// SPDX-License-Identifier: GPL-3.0-or-later
// Native reimplementation, 2026. Behavioral sources: see NOTICE.md and docs/baseline.md.
#include "core.hpp"
#include <algorithm>
#include <bitset>
#include <charconv>
#include <fstream>
#include <sstream>
namespace nand {
Error::Error(std::string m, int l, int c, std::string f) : std::runtime_error(std::move(m)), file(std::move(f)), line(l), column(c) {}
std::string readFile(const std::filesystem::path& p) {
    std::ifstream in(p, std::ios::binary);
    if (!in) throw Error("Cannot open " + p.string());
    in.seekg(0, std::ios::end); auto size = in.tellg();
    if (size < 0 || size > 32 * 1024 * 1024) throw Error("Input exceeds 32 MiB resource limit");
    std::string out(static_cast<std::size_t>(size), '\0'); in.seekg(0);
    if (!out.empty() && !in.read(out.data(), size)) throw Error("IO error while reading files");
    return out;
}
void writeFile(const std::filesystem::path& p, std::string_view data) {
    std::ofstream out(p, std::ios::binary | std::ios::trunc);
    if (!out || !out.write(data.data(), static_cast<std::streamsize>(data.size()))) throw Error("Cannot write " + p.string());
    out.close(); if (!out) throw Error("Cannot close " + p.string());
}
std::vector<std::string> lines(std::string_view s) {
    std::vector<std::string> r;
    for (std::size_t p = 0; p < s.size();) {
        auto e = s.find_first_of("\r\n", p); if(e == s.npos) e=s.size();
        r.emplace_back(s.substr(p,e-p)); p=e;
        if(p<s.size() && s[p++]=='\r' && p<s.size() && s[p]=='\n') ++p;
    } return r;
}
namespace {
std::vector<std::string> asmTokens(std::string s) {
    s.erase(std::remove(s.begin(),s.end(),' '),s.end());
    auto c=s.find("//"); if(c!=s.npos) s.resize(c);
    std::vector<std::string> r;
    auto word=[](unsigned char ch){return (ch>='a'&&ch<='z')||(ch>='A'&&ch<='Z')||(ch>='0'&&ch<='9')||std::string_view("_+-.!:&|$").find(char(ch))!=std::string_view::npos;};
    for(std::size_t i=0;i<s.size();) {
        if(s[i]=='\t'||s[i]=='\r'||s[i]=='\n'){++i;continue;}
        auto b=i++; if(word(s[b])) while(i<s.size()&&word(s[i]))++i;
        r.emplace_back(s.substr(b,i-b));
    }return r;
}
const std::map<std::string,Word> comp={
 {"0",0xea80},{"1",0xefc0},{"-1",0xee80},{"D",0xe300},{"A",0xec00},{"M",0xfc00},
 {"!D",0xe340},{"NOTD",0xe340},{"!A",0xec40},{"NOTA",0xec40},{"!M",0xfc40},{"NOTM",0xfc40},
 {"-D",0xe3c0},{"-A",0xecc0},{"-M",0xfcc0},{"D+1",0xe7c0},{"A+1",0xedc0},{"M+1",0xfdc0},
 {"D-1",0xe380},{"A-1",0xec80},{"M-1",0xfc80},{"D+A",0xe080},{"A+D",0xe080},{"D+M",0xf080},{"M+D",0xf080},
 {"D-A",0xe4c0},{"D-M",0xf4c0},{"A-D",0xe1c0},{"M-D",0xf1c0},{"D&A",0xe000},{"A&D",0xe000},
 {"D&M",0xf000},{"M&D",0xf000},{"D|A",0xe540},{"A|D",0xe540},{"D|M",0xf540},{"M|D",0xf540}};
const std::map<std::string,int> dest={{"M",8},{"D",16},{"MD",24},{"A",32},{"AM",40},{"AD",48},{"AMD",56}};
const std::map<std::string,int> jump={{"JGT",1},{"JEQ",2},{"JGE",3},{"JLT",4},{"JNE",5},{"JLE",6},{"JMP",7}};
bool shortNumber(std::string s,int& v) {
    if(s.empty())return false; if(s[0]=='+')s.erase(0,1);
    const auto result=std::from_chars(s.data(),s.data()+s.size(),v);
    return result.ec==std::errc{} && result.ptr==s.data()+s.size() && v>=-32768 && v<=32767;
}
int indexOf(std::string_view s,std::string_view prefix, int limit) {
    if(!s.starts_with(prefix)||!s.ends_with("]"))return -1;
    auto t=s.substr(prefix.size(),s.size()-prefix.size()-1);int n;
    auto r=std::from_chars(t.data(),t.data()+t.size(),n);
    if(r.ec!=std::errc{}||r.ptr!=t.data()+t.size()||n<0||n>=limit)throw Error("Illegal memory address");return n;
}
}
Assembly assemble(std::string_view source) {
    Assembly out; out.symbols={{"SP",0},{"LCL",1},{"ARG",2},{"THIS",3},{"THAT",4},{"SCREEN",16384},{"KBD",24576}};
    for(int i=0;i<16;++i)out.symbols["R"+std::to_string(i)]=Word(i);
    auto input=lines(source);std::size_t pc=0;
    for(std::size_t n=0;n<input.size();++n){auto t=asmTokens(input[n]);if(t.empty())continue;
        if(t[0]=="("){if(t.size()!=3||t[2]!=")")throw Error("')' expected",int(n+1));out.symbols[t[1]]=Word(pc);}
        else pc+=std::find(t.begin(),t.end(),"[")!=t.end()?2:1;
    }
    if(pc>32768)throw Error("Program too large"); Word next=16;
    std::function<void(std::vector<std::string>,int)> emit=[&](std::vector<std::string> t,int line){
        if(t.empty()||t[0]=="(")return;
        auto open=std::find(t.begin(),t.end(),"[");
        if(open!=t.end()){
            auto pos=static_cast<std::size_t>(open-t.begin());
            if(pos+2>=t.size()||t[pos+2]!="]"||std::count(t.begin(),t.end(),"[")!=1)throw Error("Illegal use of the [] notation",line);
            emit({"@",t[pos+1]},line);t.erase(t.begin()+pos,t.begin()+pos+3);emit(t,line);return;
        }
        Word w=0;
        if(t[0]=="@"){
            if(t.size()<2)throw Error("Unexpected end of line",line);
            if(t.size()>2)throw Error("end of line expected, '"+t[2]+"' is found",line);
            int v=0;if(shortNumber(t[1],v))w=Word(v);
            else {if(!out.symbols.contains(t[1]))out.symbols[t[1]]=next++;w=out.symbols.at(t[1]);}
        }else{
            std::size_t p=0;int d=0,j=0;
            if(t.size()>1&&t[1]=="="){if(!dest.contains(t[0]))throw Error("Destination expected",line);d=dest.at(t[0]);p=2;}
            if(p>=t.size()||!comp.contains(t[p]))throw Error("Expression expected",line);
            w=comp.at(t[p++]);
            // The legacy translator consumes one extra token for destination-less expressions.
            if(d==0&&p<t.size())++p;
            if(p<t.size()&&t[p]==";")++p;
            if(p<t.size()){if(!jump.contains(t[p]))throw Error("Jump directive expected",line);j=jump.at(t[p++]);}
            if(p<t.size())throw Error("end of line expected, '"+t[p]+"' is found",line);
            w=Word(w|d|j);
        }
        out.words.push_back(w);out.sourceLines.push_back(line);
    };
    for(std::size_t n=0;n<input.size();++n)emit(asmTokens(input[n]),int(n+1));return out;
}
std::vector<Word> parseHack(std::string_view source){
    std::vector<Word> out;int n=0;
    for(auto& line:lines(source)){++n;if(line.empty()||line.size()>16||line.find_first_not_of("01")!=line.npos)throw Error("Illegal character",n);
        Word w=0;for(char c:line)w=Word(w*2+(c-'0'));out.push_back(w);
    }if(out.size()>32768)throw Error("Program too large");return out;
}
std::string machineText(const std::vector<Word>& words){std::string out;for(auto w:words)out+=std::bitset<16>(w).to_string()+"\n";return out;}
Comparison compareText(std::string_view a,std::string_view b){
    auto x=lines(a),y=lines(b);std::size_t i=0;
    auto norm=[](std::string s){s.erase(std::remove(s.begin(),s.end(),' '),s.end());while(!s.empty()&&static_cast<unsigned char>(s.back())<=32)s.pop_back();auto p=s.find_first_not_of("\t\v\f\r\n");return p==s.npos?std::string{}:s.substr(p);};
    for(;i<std::min(x.size(),y.size());++i)if(norm(x[i])!=norm(y[i]))return {false,int(i),"Comparison failure in line "+std::to_string(i)+":\n"+norm(x[i])+"\n"+norm(y[i])+"\n"};
    if(x.size()!=y.size())return {false,int(i),std::string(x.size()<y.size()?"First":"Second")+" file is shorter (only "+std::to_string(i)+" lines)\n"};
    return {true,-1,"Comparison ended successfully\n"};
}
Cpu::Cpu(){rom.fill(0x8000);}
void Cpu::load(const std::vector<Word>& p){if(p.size()>rom.size())throw Error("Program too large");rom.fill(0x8000);std::copy(p.begin(),p.end(),rom.begin());restart();}
void Cpu::restart(){a=d=pc=0;time=0;std::fill(ram.begin()+16384,ram.begin()+24576,0);}
void Cpu::step(){
    if(pc>=rom.size())throw Error("Illegal program address");
    Word i=rom[pc];bool jumped=false;
    auto fail=[&](const std::string& m){throw Error("At line "+std::to_string(pc)+": "+m);};
    auto memory=[&](const std::string& why){if(signedWord(a)<0)fail(why+std::to_string(signedWord(a))+" is an illegal memory address.");};
    if(!(i&0x8000))a=i;
    else if((i&0xe000)==0xe000){
        Word x=d,y=a;if(i&0x1000){memory("Expression involves M but A=");y=ram[a];}
        if(i&0x800)x=0;if(i&0x400)x=Word(~x);if(i&0x200)y=0;if(i&0x100)y=Word(~y);
        Word v=(i&0x80)?Word(std::uint32_t(x)+y):Word(x&y);if(i&0x40)v=Word(~v);
        if(i&8){memory("Destination is M but A=");ram[a]=v;}
        if(i&32)a=v;if(i&16)d=v;
        int s=signedWord(v);if((s<0&&(i&4))||(s==0&&(i&2))||(s>0&&(i&1))){
            if(signedWord(a)<0)fail("Jump requested but A="+std::to_string(signedWord(a))+" is an illegal program address.");pc=a;jumped=true;
        }
    }else if(i!=0x8000)fail("Illegal instruction");
    if(!jumped){if(pc==32767)fail("Can't continue past last line");++pc;}++time;
}
int Cpu::get(std::string_view v)const{if(v=="A")return signedWord(a);if(v=="D")return signedWord(d);if(v=="PC")return pc;if(v=="time")return int(time);int n=indexOf(v,"RAM[",32768);if(n>=0)return signedWord(ram[n]);n=indexOf(v,"ROM[",32768);if(n>=0)return signedWord(rom[n]);throw Error("Unknown variable "+std::string(v));}
void Cpu::set(std::string_view v,int value){if(value>32767||value<((v=="A"||v=="PC")?0:-32768))throw Error(std::to_string(value)+" is an illegal value for: "+std::string(v));if(v=="A")a=Word(value);else if(v=="D")d=Word(value);else if(v=="PC")pc=Word(value);else{int n=indexOf(v,"RAM[",32768);if(n>=0)ram[n]=Word(value);else{n=indexOf(v,"ROM[",32768);if(n>=0)rom[n]=Word(value);else throw Error("Unknown or read-only variable "+std::string(v));}}}
}
