// SPDX-License-Identifier: GPL-3.0-or-later
#include "studio.hpp"
QVariantMap Studio::cpuInstruction()const{
    const auto pc=cpu_.pc&32767;const auto word=cpu_.rom[pc];QString decoded;
    if(!(word&0x8000))decoded="@"+QString::number(word);
    else{
        const std::map<int,QString> comp={{42,"0"},{63,"1"},{58,"-1"},{12,"D"},{48,"A"},{112,"M"},{13,"!D"},{49,"!A"},{113,"!M"},{15,"-D"},{51,"-A"},{115,"-M"},{31,"D+1"},{55,"A+1"},{119,"M+1"},{14,"D-1"},{50,"A-1"},{114,"M-1"},{2,"D+A"},{66,"D+M"},{19,"D-A"},{83,"D-M"},{7,"A-D"},{71,"M-D"},{0,"D&A"},{64,"D&M"},{21,"D|A"},{85,"D|M"}};
        const auto bits=(word>>6)&127;QString dest;if(word&32)dest+="A";if(word&16)dest+="D";if(word&8)dest+="M";
        const QStringList jumps={"","JGT","JEQ","JGE","JLT","JNE","JLE","JMP"};
        decoded=(dest.isEmpty()?QString():dest+"=")+(comp.contains(bits)?comp.at(bits):"[unknown comp]")+(word&7?";"+jumps[word&7]:QString());
    }
    bool matches=false;for(const auto* doc:docs_)if(doc->path()==cpuSourcePath_)matches=doc->text()==cpuSourceText_;
    const int line=std::size_t(pc)<cpuSourceLines_.size()?cpuSourceLines_[pc]:0;
    return {{"decoded",decoded},{"path",cpuSourcePath_},{"line",line},{"canNavigate",matches&&line>0},{"sourceChanged",!cpuSourcePath_.isEmpty()&&!matches}};
}
void Studio::setBreakpoint(int pc,bool enabled){
    if(busy_||pc<0||pc>32767||hardwareMode_)return;
    auto& points=vmMode_?vmBreakpoints_:cpuBreakpoints_;
    if(enabled)points.insert(pc);else points.remove(pc);
    emit stateChanged();
}
void Studio::addWatch(const QString& expression){
    const auto name=expression.trimmed();if(name.isEmpty()||name.size()>128||watches_.contains(name)||watches_.size()>=64)return;
    watches_.append(name);emit stateChanged();
}
void Studio::removeWatch(const QString& expression){watches_.removeAll(expression);emit stateChanged();}
QVariantList Studio::watchValues()const{
    QVariantList result;
    for(const auto& name:watches_){QVariantMap row{{"name",name}};
        try{auto value=hardwareMode_?hardware_.get(name.toStdString()):vmMode_?vm_.get(name.toStdString()):cpu_.get(name.toStdString());row["value"]=QString::number(value)+" / 0x"+QString::number(nand::Word(value),16).rightJustified(4,'0');}
        catch(const std::exception& e){row["value"]=QString::fromUtf8(e.what());}
        result.append(row);
    }
    return result;
}
