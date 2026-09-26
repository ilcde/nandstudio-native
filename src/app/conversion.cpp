// SPDX-License-Identifier: GPL-3.0-or-later
#include "studio.hpp"
#include <QFileInfo>
#include <QDirIterator>
#include <QFile>
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

void Studio::previewConversion(bool bootstrap){
    if(!current()||conversionTask_.isRunning())return;
    const auto path=current()->path(),source=current()->text();
    const auto revision=conversionRevision_;
    const auto ext=QFileInfo(path).suffix().toLower();
    std::map<std::string,std::string> buffers;
    if(ext=="hdl"||ext=="vm")for(auto* doc:docs_)if(QFileInfo(doc->path()).absolutePath()==QFileInfo(path).absolutePath()&&QFileInfo(doc->path()).suffix()==ext)buffers[QFileInfo(doc->path()).completeBaseName().toStdString()]=doc->text().toStdString();
    conversion_={{"path",path},{"status","Checking editor snapshot..."},{"busy",true}};
    emit conversionChanged();
    // A separate worker never writes artifacts or mutates the running machine.
    conversionTask_.setFuture(QtConcurrent::run([path,source,ext,buffers,bootstrap,revision]{
        TaskResult r;r.path=path;r.text=source;r.revision=revision;
        try{
            if(source.size()>1024*1024)throw nand::Error("Preview limit is 1 MiB; use the build workflow for larger files.");
            if(ext=="asm")r.artifact=QString::fromStdString(nand::machineText(nand::assemble(source.toStdString()).words));
            else if(ext=="jack")r.artifact=QString::fromStdString(nand::compileJack(source.toStdString()));
            else if(ext=="hack")r.artifact=QString::fromStdString(nand::machineText(nand::parseHack(source.toStdString())));
            else if(ext=="vm"||ext=="hdl"){
                std::map<std::string,std::string> files;qsizetype bytes=0;
                QDirIterator it(QFileInfo(path).absolutePath(),{"*."+ext},QDir::Files);
                while(it.hasNext()){
                    const auto filePath=it.next();QFile file(filePath);
                    if(!file.open(QIODevice::ReadOnly))throw nand::Error("Cannot read dependency: "+filePath.toStdString());
                    bytes+=file.size();if(bytes>4*1024*1024)throw nand::Error("Project preview limit is 4 MiB");
                    files[QFileInfo(filePath).completeBaseName().toStdString()]=file.readAll().toStdString();
                }
                for(const auto& [name,text]:buffers)files[name]=text;
                if(ext=="vm")r.artifact=QString::fromStdString(nand::translateVm(files,bootstrap));
                else{
                    nand::Hardware hardware;hardware.load(QFileInfo(path).completeBaseName().toStdString(),[&](const std::string& name)->std::optional<std::string>{const auto it=files.find(name);if(it==files.end())return std::nullopt;return it->second;});
                    r.artifact="HDL hierarchy loaded successfully. Simulation unchanged.\n";
                    for(const auto& pin:hardware.pins())r.artifact+=QString::fromStdString(pin.direction+" "+pin.name)+"["+QString::number(pin.width)+"]\n";
                }
            }
            else throw nand::Error("Preview supports ASM, Jack, VM, Hack and HDL files.");
            r.message="Valid editor snapshot. Preview only; no files written.";
        }catch(const nand::Error& e){r.error=true;r.line=e.line;r.column=e.column;r.message=QString::fromUtf8(e.what());
            if(!e.file.empty()){auto name=QString::fromStdString(e.file);if(QFileInfo(name).suffix().isEmpty())name+="."+ext;r.errorPath=QDir(QFileInfo(path).absolutePath()).filePath(name);}
        }
        catch(const std::exception& e){r.error=true;r.message=QString::fromUtf8(e.what());}
        return r;
    }));
}
