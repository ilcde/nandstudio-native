// SPDX-License-Identifier: GPL-3.0-or-later
#include "core.hpp"
#include "hdl.hpp"
#include <bitset>
#include <iomanip>
#include <regex>
#include <sstream>
namespace nand { namespace {
struct SToken {std::string text;int line;};
std::vector<SToken> scan(std::string_view s){
    std::vector<SToken> r;int line=1;
    for(std::size_t p=0;p<s.size();){char c=s[p];if(c=='\n'){++line;++p;continue;}if(c==' '||c=='\t'||c=='\r'){++p;continue;}
        if(s.substr(p,2)=="//"){while(p<s.size()&&s[p]!='\n')++p;continue;}
        if(s.substr(p,2)=="/*"){p+=2;while(p<s.size()&&s.substr(p,2)!="*/"){if(s[p++]=='\n')++line;}if(p==s.size())throw Error("Unterminated comment",line);p+=2;continue;}
        int startLine=line;std::string t;
        if(c=='"'){++p;while(p<s.size()&&s[p]!='"'){if(s[p]=='\n')++line;t+=s[p++];}if(p==s.size())throw Error("Unterminated string",line);++p;}
        else if(std::string_view(",;!{}").find(c)!=std::string_view::npos){t+=c;++p;}
        else {while(p<s.size()&&std::string_view(" \t\r\n,;!{}").find(s[p])==std::string_view::npos)t+=s[p++];}
        r.push_back({t,startLine});if(r.size()>1000000)throw Error("Script token limit exceeded");
    }return r;
}
int number(std::string s){int base=10;if(s.starts_with("%B")){base=2;s.erase(0,2);}else if(s.starts_with("%X")){base=16;s.erase(0,2);}else if(s.starts_with("%D"))s.erase(0,2);std::size_t used=0;int n;try{n=std::stoi(s,&used,base);}catch(...){throw Error("Number expected");}if(used!=s.size())throw Error("Number expected");return base==2||(base==16&&s.size()==4)?signedWord(Word(n)):n;}
struct Format {std::string name;char type;int left,width,right;};
}
ScriptResult runScript(const std::filesystem::path& path,bool useVm,std::uint64_t budget,const std::function<bool()>& cancelled){
    return runScript(path,useVm?ScriptTool::Vm:ScriptTool::Cpu,budget,cancelled);
}
ScriptResult runScript(const std::filesystem::path& path,ScriptTool tool,std::uint64_t budget,const std::function<bool()>& cancelled){
    const bool useVm=tool==ScriptTool::Vm,useHardware=tool==ScriptTool::Hardware;
    auto tokens=scan(readFile(path));Cpu cpu;Vm vm;Hardware hardware;ScriptResult result;std::vector<Format> formats;
    auto base=path.parent_path();std::filesystem::path outputPath;std::vector<std::string> expected;bool comparing=false;std::size_t cmpLine=0;std::uint64_t operations=0;
    auto get=[&](const std::string& name){return useHardware?hardware.get(name):useVm?vm.get(name):cpu.get(name);};
    auto output=[&](const std::string& row){
        if(outputPath.empty())throw Error("No output file specified");
        result.output+=row+"\n";
        if(result.output.size()>32*1024*1024)throw Error("Output resource limit exceeded");
        if(comparing){bool good=cmpLine<expected.size()&&expected[cmpLine].size()==row.size();if(good)for(std::size_t i=0;i<row.size();++i)if(expected[cmpLine][i]!='*'&&expected[cmpLine][i]!=row[i])good=false;++cmpLine;if(!good){result.passed=false;result.message="Comparison failure at line "+std::to_string(cmpLine);}}
    };
    std::function<void(std::size_t,std::size_t)> execute=[&](std::size_t begin,std::size_t end){
        for(std::size_t p=begin;p<end&&result.passed;){
            if(++operations>budget)throw Error("Script operation budget exhausted",tokens[p].line);
            if(cancelled&&cancelled())throw Error("Cancelled",tokens[p].line);
            auto command=tokens[p++];auto op=command.text;
            if(op==";"||op==","||op=="!")continue;
            std::vector<std::string> args;
            while(p<end&&tokens[p].text!=","&&tokens[p].text!=";"&&tokens[p].text!="!"&&tokens[p].text!="{")args.push_back(tokens[p++].text);
            try{
                if(op=="repeat"||op=="while"){
                    if(p==end||tokens[p].text!="{")throw Error("'{' expected");auto start=++p;while(p<end&&tokens[p].text!="}"){if(tokens[p].text=="{")throw Error("Nested Repeat and While are not allowed");++p;}if(p==end)throw Error("Repeat or While not closed");auto stop=p++;if(start==stop)throw Error("Empty loop is not allowed");
                    if(op=="repeat"){if(args.size()>1)throw Error("Invalid repeat");int count=args.empty()?0:number(args[0]);if(count<0)throw Error("Invalid repeat count");for(int i=0;(!count||i<count)&&result.passed;++i)execute(start,stop);}
                    else{if(args.size()!=3)throw Error("While condition expected");auto condition=[&](){int x=get(args[0]),y;try{y=number(args[2]);}catch(const Error&){y=get(args[2]);}auto c=args[1];if(c=="=")return x==y;if(c=="<>")return x!=y;if(c=="<")return x<y;if(c==">")return x>y;if(c=="<=")return x<=y;if(c==">=")return x>=y;throw Error("Invalid comparison operator");};while(result.passed&&condition())execute(start,stop);}
                }else if(op=="load"){
                    if(useHardware){if(args.size()!=1||!args[0].ends_with(".hdl")||args[0].find('/')!=std::string::npos||args[0].find('\\')!=std::string::npos)throw Error("HDL file name expected");hardware.loadFile(base/args[0]);}
                    else if(useVm){if(args.size()>1)throw Error("Invalid load");auto target=args.empty()?base:base/args[0];std::map<std::string,std::string> files;if(std::filesystem::is_directory(target)){for(const auto& entry:std::filesystem::directory_iterator(target))if(entry.path().extension()==".vm")files[entry.path().stem().string()]=readFile(entry.path());}else files[target.stem().string()]=readFile(target);vm.load(files);}
                    else{if(args.size()!=1)throw Error("Invalid load");auto target=base/args[0];auto text=readFile(target);cpu.load(target.extension()==".asm"?assemble(text).words:parseHack(text));}
                }else if(op=="set"){if(args.size()!=2)throw Error("Invalid set");if(useHardware)hardware.set(args[0],number(args[1]));else if(useVm)vm.set(args[0],number(args[1]));else cpu.set(args[0],number(args[1]));}
                else if(useHardware&&(op=="eval"||op=="tick"||op=="tock"||op=="ticktock")){if(!args.empty())throw Error("Invalid clock/eval command");if(op=="eval")hardware.eval();else if(op=="tick")hardware.tick();else if(op=="tock")hardware.tock();else{hardware.tick();hardware.tock();}++result.steps;}
                else if(useHardware&&args.size()==2&&args[0]=="load"){if(args.size()!=2||args[0]!="load")throw Error("ROM load expected");auto target=base/args[1];auto text=readFile(target);hardware.loadRom(op,target.extension()==".asm"?assemble(text).words:parseHack(text));}
                else if(op==(useVm?"vmstep":"ticktock")){if(!args.empty())throw Error("Invalid step");if(useVm)vm.step();else cpu.step();++result.steps;}
                else if(op=="output-file"){if(args.size()!=1)throw Error("Output file expected");outputPath=base/args[0];result.output.clear();}
                else if(op=="compare-to"){if(args.size()!=1)throw Error("Comparison file expected");expected=lines(readFile(base/args[0]));comparing=true;cmpLine=0;}
                else if(op=="output-list"){
                    formats.clear();std::string row="|";std::regex re(R"((.+)%([BDSX])(\d+)\.(\d+)\.(\d+))");
                    for(auto arg:args){if(arg.find('%')==std::string::npos)arg+="%B1.1.1";std::smatch m;if(!std::regex_match(arg,m,re))throw Error("Invalid output format");Format f{m[1],m[2].str()[0],number(m[3]),number(m[4]),number(m[5])};if(f.left<0||f.width<1||f.right<0||f.left+f.width+f.right>1000)throw Error("Output width limit exceeded");int width=f.left+f.width+f.right;auto title=f.name.substr(0,std::size_t(width));int left=(width-int(title.size()))/2;row+=std::string(left,' ')+title+std::string(width-left-title.size(),' ')+"|";formats.push_back(f);}output(row);
                }else if(op=="output"){
                    if(formats.empty())throw Error("No output list created");std::string row="|";
                    for(auto& f:formats){std::string value;if(f.type=='S'&&useHardware)value=hardware.getText(f.name);else{int n=get(f.name);value=std::to_string(n);if(f.type=='B')value=std::bitset<16>(Word(n)).to_string();else if(f.type=='X'){std::ostringstream s;s<<std::hex<<std::setw(4)<<std::setfill('0')<<Word(n);value=s.str();}}if(value.size()>std::size_t(f.width))value=value.substr(value.size()-f.width);int extra=f.width-int(value.size());row+=std::string(f.left+(f.type=='S'?0:extra),' ')+value+std::string(f.right+(f.type=='S'?extra:0),' ')+"|";}output(row);
                }else if(op=="echo"||op=="clear-echo"){} // baseline batch mode does not print echo
                else throw Error("Unsupported script command '"+op+"'");
            }catch(Error& e){e.file=path.string();e.line=command.line;throw;}
        }
    };
    auto persist=[&]{if(!outputPath.empty()){
        auto data=result.output;
#ifdef _WIN32
        std::string crlf;for(char c:data){if(c=='\n')crlf+='\r';crlf+=c;}data=std::move(crlf);
#endif
        writeFile(outputPath,data);
    }};
    try{execute(0,tokens.size());}catch(...){persist();throw;}persist();
    if(result.passed)result.message=comparing?"End of script - Comparison ended successfully":"End of script";
    return result;
}
}
