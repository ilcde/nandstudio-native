// SPDX-License-Identifier: GPL-3.0-or-later
#include "core.hpp"
#include <iostream>
int main(){
    int checks=0;auto check=[&](bool okay){if(!okay)throw std::runtime_error("Translator check "+std::to_string(checks));++checks;};
    auto execute=[&](const std::map<std::string,std::string>& files,bool boot,const std::string& stop){
        auto asmCode=nand::assemble(nand::translateVm(files,boot));nand::Cpu cpu;cpu.load(asmCode.words);cpu.ram[0]=256;cpu.ram[1]=300;cpu.ram[2]=400;cpu.ram[3]=3000;cpu.ram[4]=3010;
        const auto end=asmCode.symbols.at(stop);int count=0;while(cpu.pc!=end&&count++<1000000)cpu.step();check(cpu.pc==end);return cpu;
    };
    try{
        for(const auto& op:{"eq","gt","lt"})for(int x:{-32768,-1,0,1,32767})for(int y:{-32768,-1,0,1,32767}){
            auto value=[](int n){return n==-32768?std::string("push constant 32767\npush constant 1\nadd\n"):"push constant "+std::to_string(n<0?-n:n)+"\n"+(n<0?"neg\n":"");};
            const auto cpu=execute({{"Main",value(x)+value(y)+op+"\n"}},false,"__NAND$HALT");
            const bool expected=std::string(op)=="eq"?x==y:std::string(op)=="gt"?x>y:x<y;
            check(cpu.ram[0]==257&&cpu.ram[256]==(expected?65535:0));
        }
        const std::string segment="push constant 7\npop local 2\npush local 2\npop argument 3\npush argument 3\npop this 4\npush this 4\npop that 5\npush that 5\npop temp 6\npush temp 6\npop static 2\npush static 2\npush constant 3\nsub\npush constant 2\nand\nnot\npush constant 8\nor\npush pointer 0\npush pointer 1\nadd\n";
        std::map<std::string,std::string> files={{"Main",segment}};auto cpu=execute(files,false,"__NAND$HALT");nand::Vm vm;vm.load(files);vm.ram[0]=256;vm.ram[1]=300;vm.ram[2]=400;vm.ram[3]=3000;vm.ram[4]=3010;while(vm.pc<vm.code.size())vm.step();
        for(int address:{0,1,2,3,4,11,18,256,257,302,403,3004,3015})check(cpu.ram[address]==vm.ram[address]);
        files={{"Sys","function Sys.init 0\npush constant 6\ncall Sum.down 1\npop temp 0\nlabel END\ngoto END\n"},{"Sum","function Sum.down 1\npush argument 0\npush constant 0\neq\nif-goto ZERO\npush argument 0\npush argument 0\npush constant 1\nsub\ncall Sum.down 1\nadd\nreturn\nlabel ZERO\npush constant 0\nreturn\n"}};
        cpu=execute(files,true,"Sys.init$END");check(cpu.ram[5]==21&&cpu.ram[0]==261);
        files={{"A","push constant 12\npop static 0\n"},{"B","push constant 34\npop static 0\npush static 0\n"}};
        cpu=execute(files,false,"__NAND$HALT");check(cpu.ram[16]==12&&cpu.ram[17]==34&&cpu.ram[256]==34);
        for(const auto& bad:{"pop constant 0","push pointer 2","call Missing.f 0","function SP 0","goto missing"}){bool rejected=false;try{nand::translateVm({{"Main",bad}});}catch(const nand::Error&){rejected=true;}check(rejected);}
        bool rejected=false;try{nand::translateVm({{"Main","push constant 0"}},true);}catch(const nand::Error&){rejected=true;}check(rejected);
        std::cout<<checks<<" translator checks passed\n";
    }catch(const std::exception& e){std::cerr<<e.what()<<"\n";return 1;}
}
