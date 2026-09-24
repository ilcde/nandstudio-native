// SPDX-License-Identifier: GPL-3.0-or-later
#include "core.hpp"
#include <cctype>
#include <set>
namespace nand { namespace {
struct Token { std::string text; int line, column; bool string = false; };
std::vector<Token> tokenize(std::string_view s){
    std::vector<Token> out;std::size_t p=0;int line=1,col=1;
    auto advance=[&](){char c=s[p++];if(c=='\n'){++line;col=1;}else ++col;return c;};
    while(p<s.size()){
        if(std::isspace(static_cast<unsigned char>(s[p]))){advance();continue;}
        if(s.substr(p,2)=="//"){while(p<s.size()&&s[p]!='\n')advance();continue;}
        if(s.substr(p,2)=="/*"){advance();advance();while(p<s.size()&&s.substr(p,2)!="*/")advance();if(p==s.size())throw Error("Unterminated comment",line,col);advance();advance();continue;}
        Token t{{},line,col};char c=advance();
        if(c=='"'){t.string=true;while(p<s.size()&&s[p]!='"'&&s[p]!='\n')t.text+=advance();if(p==s.size()||s[p]!='"')throw Error("Unterminated string",t.line,t.column);advance();}
        else if(std::isalnum(static_cast<unsigned char>(c))||c=='_'){t.text+=c;while(p<s.size()&&(std::isalnum(static_cast<unsigned char>(s[p]))||s[p]=='_'))t.text+=advance();}
        else if(std::string_view("{}()[].,;+-*/&|<>=~").find(c)!=std::string_view::npos)t.text+=c;
        else throw Error("Unexpected character",t.line,t.column);
        out.push_back(std::move(t));if(out.size()>1000000)throw Error("Token resource limit exceeded");
    }out.push_back({"",line,col});return out;
}
struct Symbol { std::string type, segment; int index; };
class Compiler {
    std::vector<Token> tokens;std::size_t p=0;int depth=0;
    std::map<std::string,Symbol> fields,locals;
    std::map<std::string,int> count;std::map<std::string,std::string> routines;
    std::string cls,kind,result;int ifs=0,whiles=0;
    const Token& peek()const{return tokens.at(p);}
    bool at(std::string_view s)const{return !peek().string&&peek().text==s;}
    [[noreturn]]void fail(const std::string& m)const{throw Error(m,peek().line,peek().column);}
    bool take(std::string_view s){if(at(s)){++p;return true;}return false;}
    void expect(std::string_view s){if(!take(s))fail("Expected '"+std::string(s)+"'");}
    std::string identifier(){
        static const std::set<std::string> reserved={"class","constructor","function","method","field","static","var","int","char","boolean","void","true","false","null","this","let","do","if","else","while","return"};
        auto t=peek();if(t.string||t.text.empty()||!(std::isalpha(static_cast<unsigned char>(t.text[0]))||t.text[0]=='_')||reserved.contains(t.text))fail("Identifier expected");++p;return t.text;
    }
    std::string type(bool allowVoid=false){if(at("int")||at("char")||at("boolean")||(allowVoid&&at("void")))return tokens[p++].text;return identifier();}
    void emit(const std::string& text){result+=text+"\n";}
    void push(const std::string& seg,int n){emit("push "+seg+" "+std::to_string(n));}
    Symbol symbol(const std::string& name){if(locals.contains(name))return locals.at(name);if(fields.contains(name))return fields.at(name);fail("Unknown variable '"+name+"'");}
    bool known(const std::string& name)const{return locals.contains(name)||fields.contains(name);}
    void access(const std::string& op,const Symbol& s){emit(op+" "+s.segment+" "+std::to_string(s.index));}
    void declarations(const std::string& seg,std::map<std::string,Symbol>& table){auto typ=type();do{auto name=identifier();if(table.contains(name))fail("Duplicate variable '"+name+"'");table[name]={typ,seg,count[seg]++};}while(take(","));expect(";");}
    void call(std::string name){
        int args=0;std::string target;
        if(take(".")){if(known(name)){auto s=symbol(name);access("push",s);target=s.type;args=1;}else target=name;target+="."+identifier();}
        else{target=cls+"."+name;if(!routines.contains(name)||routines[name]=="method"){push("pointer",0);args=1;}}
        expect("(");if(!at(")")){do{expression();++args;}while(take(","));}expect(")");emit("call "+target+" "+std::to_string(args));
    }
    void term(){
        if(++depth>256)fail("Expression nesting limit exceeded");
        auto t=peek();
        if(take("(")){expression();expect(")");}
        else if(take("-")){term();emit("neg");}
        else if(take("~")){term();emit("not");}
        else if(t.string){++p;push("constant",int(t.text.size()));emit("call String.new 1");for(unsigned char c:t.text){push("constant",c);emit("call String.appendChar 2");}}
        else if(!t.text.empty()&&std::isdigit(static_cast<unsigned char>(t.text[0]))){++p;int value=0;for(char c:t.text){if(c<'0'||c>'9'||value>3276)fail("Integer constant out of range");value=value*10+c-'0';}if(value>32767)fail("Integer constant out of range");push("constant",value);}
        else if(take("true")){push("constant",0);emit("not");}
        else if(take("false")||take("null"))push("constant",0);
        else if(take("this"))push("pointer",0);
        else {auto name=identifier();if(at("(")||at("."))call(name);else{auto s=symbol(name);if(take("[")){expression();expect("]");access("push",s);emit("add");emit("pop pointer 1");push("that",0);}else access("push",s);}}
        --depth;
    }
    void expression(){term();static const std::map<std::string,std::string> ops={{"+","add"},{"-","sub"},{"*","call Math.multiply 2"},{"/","call Math.divide 2"},{"&","and"},{"|","or"},{"<","lt"},{">","gt"},{"=","eq"}};while(ops.contains(peek().text)&&!peek().string){auto op=tokens[p++].text;term();emit(ops.at(op));}}
    void statements(){
        if(++depth>256)fail("Statement nesting limit exceeded");
        while(!at("}")&&!peek().text.empty()){
            if(take("let")){auto s=symbol(identifier());bool array=take("[");if(array){expression();expect("]");access("push",s);emit("add");}expect("=");expression();expect(";");if(array){emit("pop temp 0");emit("pop pointer 1");push("temp",0);emit("pop that 0");}else access("pop",s);}
            else if(take("do")){call(identifier());expect(";");emit("pop temp 0");}
            else if(take("return")){if(at(";"))push("constant",0);else expression();expect(";");emit("return");}
            else if(take("if")){int id=ifs++;auto n=std::to_string(id);expect("(");expression();expect(")");emit("if-goto IF_TRUE"+n);emit("goto IF_FALSE"+n);emit("label IF_TRUE"+n);expect("{");statements();expect("}");if(take("else")){emit("goto IF_END"+n);emit("label IF_FALSE"+n);expect("{");statements();expect("}");emit("label IF_END"+n);}else emit("label IF_FALSE"+n);}
            else if(take("while")){auto n=std::to_string(whiles++);emit("label WHILE_EXP"+n);expect("(");expression();expect(")");emit("not");emit("if-goto WHILE_END"+n);expect("{");statements();expect("}");emit("goto WHILE_EXP"+n);emit("label WHILE_END"+n);}
            else fail("Statement expected");
        }--depth;
    }
public:
    explicit Compiler(std::string_view s):tokens(tokenize(s)){}
    std::string run(){
        expect("class");cls=identifier();expect("{");
        // Pre-scan declarations so unqualified function calls do not acquire a hidden receiver.
        int nesting=1;for(std::size_t i=p;i+3<tokens.size();++i){if(tokens[i].text=="{")++nesting;if(tokens[i].text=="}")--nesting;if(nesting==1&&(tokens[i].text=="method"||tokens[i].text=="function"||tokens[i].text=="constructor"))routines[tokens[i+2].text]=tokens[i].text;}
        while(at("static")||at("field")){auto seg=take("static")?"static":"this";if(std::string_view(seg)=="this")expect("field");declarations(seg,fields);}
        while(!at("}")){
            if(!(at("method")||at("constructor")||at("function")))fail("Subroutine declaration expected");kind=tokens[p++].text;type(true);auto name=identifier();locals.clear();count["argument"]=kind=="method"?1:0;count["local"]=0;ifs=whiles=0;
            expect("(");if(!at(")")){do{auto typ=type();auto arg=identifier();locals[arg]={typ,"argument",count["argument"]++};}while(take(","));}expect(")");expect("{");while(take("var"))declarations("local",locals);
            emit("function "+cls+"."+name+" "+std::to_string(count["local"]));
            if(kind=="method"){push("argument",0);emit("pop pointer 0");}else if(kind=="constructor"){push("constant",count["this"]);emit("call Memory.alloc 1");emit("pop pointer 0");}
            statements();expect("}");
        }expect("}");if(!peek().text.empty())fail("Unexpected text after class");return result;
    }
};
}std::string compileJack(std::string_view source){return Compiler(source).run();}}
