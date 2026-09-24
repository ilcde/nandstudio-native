#include "core.hpp"
#include <iostream>
#include <random>
int main(){int checks=0;auto check=[&](bool b){++checks;if(!b)throw std::runtime_error("Check failed: "+std::to_string(checks));};
    try{
        auto a=nand::assemble("@2\nD=A\n@3\nD=D+A\n@0\nM=D\n");check(a.words==std::vector<nand::Word>{2,0xec10,3,0xe090,0,0xe308});
        nand::Cpu c;c.load(a.words);for(int i=0;i<6;++i)c.step();check(c.ram[0]==5&&c.time==6);
        c.load(nand::assemble("@7\nAMD=1;JMP").words);c.step();c.step();check(c.pc==1&&c.a==1&&c.d==1&&c.ram[7]==1);
        c.ram[0]=15;c.ram[16384]=1;c.restart();check(c.ram[0]==15&&c.ram[16384]==0);
        check(nand::assemble("D=M[21]").words==std::vector<nand::Word>{21,0xfc10});
        check(nand::assemble("@32768\n@-1\nD=NOTD").words==std::vector<nand::Word>{16,65535,0xe350});
        check(nand::compareText("a b\r\nc", "ab\nc").equal);check(!nand::compareText("a\tb", "ab").equal);check(nand::compareText("x","y").line==0);
        nand::Vm vm;vm.load({{"Main","push constant 32767\npush constant 1\nadd\npush constant 1\nlt\n"}});vm.ram[0]=256;for(int i=0;i<5;++i)vm.step();check(vm.ram[256]==65535);
        auto code=nand::compileJack("class Main { function int f(int a) { return a+1; } }");check(code=="function Main.f 0\npush argument 0\npush constant 1\nadd\nreturn\n");
        bool rejected=false;try{nand::compileJack("class X { function void f() { let");}catch(const nand::Error&){rejected=true;}check(rejected);
        std::mt19937 rng(42);for(int n=0;n<2000;++n){c.a=nand::Word(rng());c.d=nand::Word(rng());auto x=c.d,y=c.a;c.rom[0]=0xe090;c.pc=0;c.step();check(c.d==nand::Word(std::uint32_t(x)+y));}
        std::cout<<checks<<" checks passed\n";
    }catch(const std::exception& e){std::cerr<<e.what()<<"\n";return 1;}return 0;
}
