// SPDX-License-Identifier: GPL-3.0-or-later
#include "studio.hpp"
#include <QFileInfo>
#include <QtConcurrent>

QVariantMap Studio::convertWord(const QString& text,int base)const{
    if(base!=2&&base!=10&&base!=16)return {{"error","Choose decimal, binary or hexadecimal."}};
    bool valid=false;
    const auto trimmed=text.trimmed();
    auto value=trimmed.toLongLong(&valid,base);
    if(!valid||trimmed.isEmpty()||value < -32768||value>65535)
        return {{"error","Enter a 16-bit value (-32768 to 65535) in the selected base."}};
    const auto word=nand::Word(value);
    return {{"binary",QString::number(word,2).rightJustified(16,'0')},
            {"hex",QString::number(word,16).rightJustified(4,'0')},
            {"signed",nand::signedWord(word)},{"unsigned",int(word)}};
}

void Studio::previewConversion(){
    if(!current()||conversionTask_.isRunning())return;
    const auto path=current()->path(),source=current()->text();
    const auto ext=QFileInfo(path).suffix().toLower();
    conversion_={{"path",path},{"status","Checking editor snapshot..."},{"busy",true}};
    emit conversionChanged();
    // A separate worker never writes artifacts or mutates the running machine.
    conversionTask_.setFuture(QtConcurrent::run([path,source,ext]{
        TaskResult r;r.path=path;r.text=source;
        try{
            if(source.size()>1024*1024)throw nand::Error("Preview limit is 1 MiB; use the build workflow for larger files.");
            if(ext=="asm")r.artifact=QString::fromStdString(nand::machineText(nand::assemble(source.toStdString()).words));
            else if(ext=="jack")r.artifact=QString::fromStdString(nand::compileJack(source.toStdString()));
            else if(ext=="hack")r.artifact=QString::fromStdString(nand::machineText(nand::parseHack(source.toStdString())));
            else throw nand::Error("Preview supports ASM to Hack, Jack to VM, and Hack validation.");
            r.message="Valid editor snapshot. Preview only; no files written.";
        }catch(const nand::Error& e){r.error=true;r.line=e.line;r.column=e.column;r.message=QString::fromUtf8(e.what());}
        catch(const std::exception& e){r.error=true;r.message=QString::fromUtf8(e.what());}
        return r;
    }));
}
