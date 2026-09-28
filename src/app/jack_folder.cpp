// SPDX-License-Identifier: GPL-3.0-or-later
#include "studio.hpp"
#include <QDir>
#include <QFile>
#include <QSaveFile>
#include <QtConcurrent>
namespace {
QByteArray readBuildFile(const QString& path) {
    QFile file(path);
    if(!file.open(QIODevice::ReadOnly))throw nand::Error("Cannot read: "+path.toStdString());
    if(file.size()>16*1024*1024)throw nand::Error("Folder build file exceeds 16 MiB: "+path.toStdString());
    return file.readAll();
}
void checkOutput(const GeneratedFile& file) {
    if(QFileInfo(file.path).isSymLink())throw nand::Error("Generated output is a symbolic link: "+file.path.toStdString());
    if(QFileInfo::exists(file.path)!=file.existed || (file.existed&&readBuildFile(file.path)!=file.previous))
        throw nand::Error("Generated output changed externally: "+file.path.toStdString());
}
}
void Studio::buildJackFolder() {
    if(busy_||!current())return;
    const auto folder=QFileInfo(current()->path()).absolutePath();
    QMap<QString,QByteArray> buffers;
    for(auto* doc:docs_)if(QFileInfo(doc->path()).absolutePath()==folder&&QFileInfo(doc->path()).suffix()=="jack")
        buffers.insert(doc->path(),doc->text().toUtf8());
    busy_=true;cancelled_=false;
    log("Compiling Jack folder snapshot (open buffers take precedence): "+folder);emit stateChanged();
    task_.setFuture(QtConcurrent::run([this,folder,buffers] {
        TaskResult result;
        try {
            auto names=QDir(folder).entryList({"*.jack"},QDir::Files,QDir::Name);
            if(names.isEmpty())throw nand::Error("No .jack files in the active document folder");
            if(names.size()>1024)throw nand::Error("Folder build is limited to 1024 source files");
            qsizetype total=0, retained=0;
            for(const auto& name:names) {
                if(cancelled_)throw nand::Error("Folder build cancelled; no outputs written");
                result.path=QDir(folder).filePath(name);
                const auto source=buffers.contains(result.path)?buffers[result.path]:readBuildFile(result.path);
                total+=source.size();if(total>16*1024*1024)throw nand::Error("Folder source snapshot exceeds 16 MiB");
                GeneratedFile output;output.path=QDir(folder).filePath(QFileInfo(name).completeBaseName()+".vm");
                if(QFileInfo(output.path).isSymLink())throw nand::Error("Generated output is a symbolic link");
                output.existed=QFileInfo::exists(output.path);
                if(output.existed)output.previous=readBuildFile(output.path);
                output.bytes=QByteArray::fromStdString(nand::compileJack(source.toStdString()));
#ifdef Q_OS_WIN
                output.bytes.replace("\n","\r\n");
#endif
                retained+=output.previous.size()+output.bytes.size();
                if(retained>64*1024*1024)throw nand::Error("Folder output snapshot exceeds 64 MiB");
                result.generated.append(std::move(output));
            }
            result.message="Jack folder compiled";
        } catch(const nand::Error& error) {
            result.error=true;result.line=error.line;result.column=error.column;result.message=QString::fromUtf8(error.what());
        } catch(const std::exception& error) { result.error=true;result.message=QString::fromUtf8(error.what()); }
        return result;
    }));
}
void Studio::finishJackFolder(TaskResult& result) {
    if(result.error||result.generated.isEmpty())return;
    QStringList written;
    try {
        if(cancelled_)throw nand::Error("Folder build cancelled; no outputs written");
        // This runs on the GUI thread after compilation. Catch edits made while
        // the worker ran, and validate every target before committing the first.
        for(const auto& file:result.generated) {
            for(auto* doc:docs_)if(doc->path()==file.path&&doc->dirty())
                throw nand::Error("Generated output has unsaved edits: "+file.path.toStdString());
            checkOutput(file);
        }
        for(const auto& file:result.generated) {
            checkOutput(file);
            QSaveFile output(file.path);output.setDirectWriteFallback(false);
            if(!output.open(QIODevice::WriteOnly)||output.write(file.bytes)!=file.bytes.size()||!output.commit())
                throw nand::Error("Cannot commit generated file: "+file.path.toStdString());
            written.append(file.path);log("Generated: "+file.path);
            for(auto* doc:docs_)if(doc->path()==file.path) {
                doc->original=file.bytes;doc->text_=QString::fromUtf8(file.bytes).replace("\r\n","\n");
                emit doc->textChanged();emit doc->changed();
            }
        }
        result.message=QString("Jack folder build succeeded: %1 VM files; source buffers were not saved.").arg(written.size());
        result.artifact=written.first();refreshWorkspace();
    } catch(const std::exception& error) {
        result.error=true;result.message=QString::fromUtf8(error.what());
        result.message+=written.isEmpty()?"; no outputs written":"; committed before failure: "+written.join(", ");
    }
}
