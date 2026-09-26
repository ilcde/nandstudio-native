// SPDX-License-Identifier: GPL-3.0-or-later
#include "studio.hpp"
#include "storage.hpp"
#include <QUuid>
#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <QDirIterator>
#include <QSaveFile>
#include <QStandardPaths>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QSyntaxHighlighter>
#include <QRegularExpression>
#include <QStringDecoder>
#include <QtConcurrent>
#include <QUrl>
#include <QTextStream>
#include <QCoreApplication>
namespace {
QByteArray read(const QString& p){return storage::provider(QUrl::fromLocalFile(p)).read(QUrl::fromLocalFile(p));}
bool atomicSave(const QString& p,const QByteArray& bytes){try{storage::provider(QUrl::fromLocalFile(p)).write(QUrl::fromLocalFile(p),bytes);return true;}catch(...){return false;}}
class Highlight : public QSyntaxHighlighter {
public:using QSyntaxHighlighter::QSyntaxHighlighter;
protected:void highlightBlock(const QString& text)override{
    QTextCharFormat keyword,number,comment,string;keyword.setForeground(QColor("#47a6ef"));number.setForeground(QColor("#d79c56"));comment.setForeground(QColor("#7c9979"));string.setForeground(QColor("#d18ab8"));
    static const QRegularExpression words(R"(\b(class|constructor|function|method|field|static|var|int|char|boolean|void|true|false|null|this|let|do|if|else|while|return|CHIP|IN|OUT|PARTS|BUILTIN|CLOCKED|push|pop|add|sub|neg|eq|gt|lt|and|or|not|label|goto|call|local|argument|constant|pointer|temp|set|load|repeat|output|tick|tock|ticktock|vmstep|eval|JGT|JEQ|JGE|JLT|JNE|JLE|JMP)\b)");
    auto apply=[&](const QRegularExpression& re,const QTextCharFormat& f){auto it=re.globalMatch(text);while(it.hasNext()){auto m=it.next();setFormat(int(m.capturedStart()),int(m.capturedLength()),f);}};
    apply(words,keyword);apply(QRegularExpression(R"(\b\d+\b)"),number);apply(QRegularExpression("\"[^\"]*\""),string);
    int start=previousBlockState()==1?0:int(text.indexOf("/*"));setCurrentBlockState(0);
    while(start>=0){int end=int(text.indexOf("*/",start+(previousBlockState()==1&&start==0?0:2)));if(end<0){setFormat(start,int(text.size())-start,comment);setCurrentBlockState(1);break;}setFormat(start,end+2-start,comment);start=int(text.indexOf("/*",end+2));}
    int slash=int(text.indexOf("//"));if(slash>=0)setFormat(slash,int(text.size())-slash,comment);
}};
QString sessionPath(){auto dir=qEnvironmentVariable("NAND_TEST_STATE_DIR");if(dir.isEmpty())dir=QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);QDir().mkpath(dir);return dir+"/recovery.json";}
}
Document::Document(QString path,QObject* parent):QObject(parent),path_(std::move(path)){
    original=read(path_);auto bytes=original;bom=bytes.startsWith("\xef\xbb\xbf");if(bom)bytes.remove(0,3);
    QStringDecoder decoder(QStringDecoder::Utf8);text_=decoder(bytes);if(decoder.hasError())throw std::runtime_error("Unsupported text encoding: UTF-8 required; file has not been modified");
    if(text_.contains("\r\n"))newline="\r\n";else if(text_.contains('\r'))newline="\r";text_.replace("\r\n","\n");text_.replace('\r','\n');
}
QString Document::name()const{return QFileInfo(path_).fileName();}
void Document::setText(const QString& t){if(t==text_)return;text_=t;dirty_=true;emit textChanged();emit changed();}
bool Document::save(){
    if(!dirty_)return true;
    try{auto disk=read(path_);if(disk!=original){conflictingBytes_=disk;conflictPending_=true;emit changed();emit conflict();emit error("Save conflict: the file changed outside NandStudio. Your buffer is retained in recovery. Open the disk copy separately and reconcile before saving.");return false;}}
    catch(const std::exception& e){emit error(QString::fromUtf8(e.what()));return false;}
    auto text=text_;text.replace("\n",newline);QByteArray bytes=text.toUtf8();if(bom)bytes.prepend("\xef\xbb\xbf");
    if(!atomicSave(path_,bytes)){emit error("Save failed; buffer retained");return false;}original=bytes;dirty_=false;conflictPending_=false;emit changed();return true;
}
bool Document::resolveConflict(const QString& action){
    try{auto disk=read(path_);if(!conflictPending_)return false;if(disk!=conflictingBytes_){conflictingBytes_=disk;emit changed();emit error("Disk changed again; review the new version before resolving");return false;}
        if(action=="reload"){QStringDecoder decoder(QStringDecoder::Utf8);auto bytes=disk;if(bytes.startsWith("\xef\xbb\xbf"))bytes.remove(0,3);QString text=decoder(bytes);if(decoder.hasError())throw std::runtime_error("Disk file is not UTF-8");original=disk;bom=disk.startsWith("\xef\xbb\xbf");newline=text.contains("\r\n")?"\r\n":text.contains('\r')?"\r":"\n";text.replace("\r\n","\n");text.replace('\r','\n');text_=text;dirty_=false;conflictPending_=false;emit textChanged();emit changed();return true;}
        if(action=="overwrite"){auto previous=original;original=disk;if(save()){conflictPending_=false;return true;}original=previous;}return false;
    }catch(const std::exception& e){emit error(e.what());return false;}
}
void Document::highlight(QQuickTextDocument* d){if(d&&d->textDocument())new Highlight(d->textDocument());}
Studio::Studio(){
    connect(this,&Studio::hardwareSourceChanged,this,[this]{
        ++conversionRevision_;
        if(conversion_.isEmpty())return;
        conversion_["stale"]=true;conversion_["output"]=QString();conversion_["error"]=false;
        conversion_["status"]="Editor changed; request a fresh preview.";
        emit conversionChanged();
    });
    connect(&conversionTask_,&QFutureWatcher<TaskResult>::finished,this,[this]{
        const auto r=conversionTask_.result();
        const bool stale=r.revision!=conversionRevision_||!current()||current()->path()!=r.path||current()->text()!=r.text;
        conversion_={{"path",r.errorPath.isEmpty()?r.path:r.errorPath},{"busy",false},{"stale",stale},{"error",r.error},
                     {"line",r.line},{"column",r.column},{"output",stale?QString():r.artifact},
                     {"status",stale?QString("Editor changed; request a fresh preview."):r.message}};
        if(!stale){
            for(qsizetype i=diagnostics_.size();i-->0;){const auto entry=diagnostics_[i].toMap();if(entry["source"]=="preview"&&entry["owner"]==r.path)diagnostics_.removeAt(i);}
            if(r.error)diagnostics_.append(QVariantMap{{"source","preview"},{"owner",r.path},{"path",r.errorPath.isEmpty()?r.path:r.errorPath},{"line",r.line},{"column",r.column},{"message",r.message}});
            emit diagnosticsChanged();
        }
        emit conversionChanged();
    });
    connect(&transfer_,&QFutureWatcher<TaskResult>::finished,this,[this]{auto r=transfer_.result();busy_=false;log(r.message);if(!r.error&&!r.path.isEmpty())openWorkspace(QUrl::fromLocalFile(r.path));emit stateChanged();});
    connect(&task_,&QFutureWatcher<TaskResult>::finished,this,[this]{busy_=false;auto r=task_.result();log(r.message);if(r.error){diagnostics_.append(QVariantMap{{"path",r.path},{"line",r.line},{"column",r.column},{"message",r.message}});emit diagnosticsChanged();for(int i=0;i<docs_.size();++i)if(docs_[i]->path()==r.path){emit diagnostic(i,r.line,r.column,r.message);break;}}else if(!r.artifact.isEmpty()){log("Generated: "+r.artifact);for(auto* d:docs_)if(d->path()==r.artifact&&!d->dirty()){try{auto bytes=read(r.artifact);d->original=bytes;d->text_=QString::fromUtf8(bytes).replace("\r\n","\n");emit d->textChanged();emit d->changed();}catch(const std::exception& e){log(e.what());}}open(QUrl::fromLocalFile(r.artifact));}emit stateChanged();});
    connect(&execution_,&QFutureWatcher<std::shared_ptr<ExecutionResult>>::finished,this,[this]{
        busy_=false;auto r=execution_.result();
        pauseReason_=r->pauseReason;
        if(r->error.isEmpty()||!hardwareEvent_.startsWith("load")){
            cpu_=std::move(r->cpu);vm_=std::move(r->vm);hardware_=std::move(r->hardware);vmMode_=r->vmMode;hardwareMode_=r->hardwareMode;
            hardwarePath_=r->hardwarePath;hardwareSources_=r->hardwareSources;hardwareMessage_=r->hardwareMessage;
            if(r->error.isEmpty()&&hardwareMode_&&!hardwareEvent_.isEmpty()){if(hardwareEvent_.startsWith("load"))hardwareTrace_.clear();recordHardware(hardwareEvent_);}
        }
        if(!r->error.isEmpty()){
            hardwareMessage_=r->error;log(r->error);
            if(!hardwareEvent_.isEmpty())hardwareEvaluation_="Hardware action failed: "+r->error+(hardwareEvent_.startsWith("load")?". Previous simulation retained.":"");
            if(!r->errorPath.isEmpty()){diagnostics_.append(QVariantMap{{"path",r->errorPath},{"line",r->errorLine},{"column",r->errorColumn},{"message",r->error}});emit diagnosticsChanged();for(int i=0;i<docs_.size();++i)if(docs_[i]->path()==r->errorPath){emit diagnostic(i,r->errorLine,r->errorColumn,r->error);break;}}
        }
        const bool loaded=r->error.isEmpty()&&hardwareEvent_.startsWith("load");
        if(loaded&&hardwareEvent_=="load")hardwareEvaluation_.clear();
        if(r->error.isEmpty()&&hardwareMode_&&(hardwareEvent_=="eval"||hardwareEvent_=="load-eval")){
            QStringList values;for(const auto& pin:hardware_.pins())if(pin.direction=="output")values.append(QString::fromStdString(pin.name)+"="+QString::number(nand::signedWord(pin.value)));
            hardwareEvaluation_="Eval completed: "+values.join(", ");
            log(hardwareEvaluation_);
        }
        hardware_.keyboard(nand::Word(keyboard_.load()));(vmMode_?vm_.ram:cpu_.ram)[24576]=nand::Word(keyboard_.load());hardwareEvent_.clear();emit stateChanged();emit hardwareSourceChanged();if(loaded)emit hardwareLoaded();
    });
    connect(this,&Studio::activeChanged,this,&Studio::hardwareSourceChanged);
    connect(&workspaceTask_,&QFutureWatcher<WorkspaceResult>::finished,this,[this]{auto r=workspaceTask_.result();if(r.path!=workspace_)return;if(!r.error.isEmpty())log(r.error);files_=r.files;emit filesChanged();});
    connect(&searchTask_,&QFutureWatcher<QVariantList>::finished,this,[this]{searchResults_=searchTask_.result();emit searchChanged();log(QString::number(searchResults_.size())+" search matches");});
    connect(&autosaveTimer_,&QTimer::timeout,this,[this]{if(!busy_)for(auto* d:docs_)if(d->dirty()&&!d->hasConflict())d->save();});
    connect(&recoveryTimer_,&QTimer::timeout,this,&Studio::saveSession);recoveryTimer_.start(5000);if(!QCoreApplication::arguments().contains("--self-test"))recover();
    sessionDebounce_.setSingleShot(true);sessionDebounce_.setInterval(250);
    connect(&sessionDebounce_,&QTimer::timeout,this,&Studio::saveSession);
    connect(this,&Studio::activeChanged,this,&Studio::saveSession);
    connect(this,&Studio::documentsChanged,this,&Studio::saveSession);
    connect(this,&Studio::filesChanged,this,&Studio::saveSession);
}
Studio::~Studio(){cancelled_=true;conversionTask_.waitForFinished();task_.waitForFinished();execution_.waitForFinished();workspaceTask_.waitForFinished();searchTask_.waitForFinished();transfer_.waitForFinished();saveSession();}
QVariantList Studio::documents()const{QVariantList r;for(auto* d:docs_)r.append(QVariant::fromValue(d));return r;}
Document* Studio::current()const{return active_>=0&&active_<docs_.size()?docs_[active_]:nullptr;}
void Studio::setActive(int i){if(i>=0&&i<docs_.size()&&active_!=i){active_=i;emit activeChanged();}}
void Studio::log(QString s){output_+=s+'\n';if(output_.size()>100000)output_=output_.right(100000);emit outputChanged();}
void Studio::open(const QUrl& url){
    if(!url.isLocalFile()){log("This build does not yet implement Android document-provider URIs. Import/export and persistable grants are release blockers.");return;}
    auto path=QFileInfo(url.toLocalFile()).absoluteFilePath();for(int i=0;i<docs_.size();++i)if(docs_[i]->path()==path){setActive(i);return;}
    try{auto* d=new Document(path,this);connect(d,&Document::error,this,&Studio::log);connect(d,&Document::conflict,this,[this,d]{emit conflict(d);});connect(d,&Document::textChanged,this,&Studio::hardwareSourceChanged);connect(d,&Document::textChanged,&sessionDebounce_,qOverload<>(&QTimer::start));docs_.append(d);active_=int(docs_.size())-1;emit documentsChanged();emit activeChanged();}catch(const std::exception& e){log(QString::fromUtf8(e.what()));}
}
void Studio::openWorkspace(const QUrl& url){
    if(!url.isLocalFile()){emit workspaceImportRequested(url);return;}
    auto path=QFileInfo(url.toLocalFile()).absoluteFilePath();if(!QFileInfo(path).isDir()){log("Workspace must be a folder");return;}workspace_=path;refreshWorkspace();
}
void Studio::importWorkspace(const QUrl& url){
    if(busy_)return;auto parent=QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)+"/workspaces";
    if(!QDir().mkpath(parent)){log("Cannot create local workspace storage");return;}
    auto name="Project-"+QUuid::createUuid().toString(QUuid::WithoutBraces);cancelled_=false;busy_=true;emit stateChanged();
    transfer_.setFuture(QtConcurrent::run([this,url,parent,name]{TaskResult r;try{auto copy=storage::copyWorkspace(url,QUrl::fromLocalFile(parent),name,[this]{return cancelled_.load();});r.path=copy.toLocalFile();r.message="Imported separate editable local copy: "+r.path+". Save changes this local copy only; use Export workspace copy to share results. Original folder unchanged.";}catch(const std::exception& e){r.error=true;r.message=e.what();}return r;}));
}
void Studio::exportWorkspace(const QUrl& destination){
    if(busy_||workspace_.isEmpty())return;if(hasDirtyDocuments()){log("Save open documents before exporting; unsaved buffers were not exported");return;}
    auto source=QUrl::fromLocalFile(workspace_);auto name="NandStudio-export-"+QUuid::createUuid().toString(QUuid::WithoutBraces);cancelled_=false;busy_=true;emit stateChanged();
    transfer_.setFuture(QtConcurrent::run([this,source,destination,name]{TaskResult r;try{auto copy=storage::copyWorkspace(source,destination,name,[this]{return cancelled_.load();});r.message="Exported saved workspace as a new folder: "+copy.toString()+". Existing provider files were not overwritten.";}catch(const std::exception& e){r.error=true;r.message=e.what();}return r;}));
}
void Studio::refreshWorkspace(){
    auto path=workspace_;if(path.isEmpty())return;emit filesChanged();workspaceTask_.setFuture(QtConcurrent::run([path]{WorkspaceResult result;result.path=path;try{
        QList<QUrl> pending{QUrl::fromLocalFile(path)};while(!pending.isEmpty()&&result.files.size()<10000){auto folder=pending.takeLast();for(auto& entry:storage::provider(folder).list(folder)){if(entry.directory){if(entry.name.startsWith('.')||entry.name.startsWith("build")||entry.name=="reference"||entry.name=="dist")continue;pending.append(entry.url);}else{auto ext=QFileInfo(entry.name).suffix().toLower();if(!QStringList{"hdl","jack","asm","hack","vm","tst","cmp","out","xml","txt","md","dat"}.contains(ext))continue;result.files.append(QVariantMap{{"path",entry.url},{"name",QDir(path).relativeFilePath(entry.url.toLocalFile())}});}}}
    }catch(const std::exception& e){result.error=e.what();}return result;}));
}
bool Studio::createFile(const QString& relative){try{auto target=storage::workspaceChild(QUrl::fromLocalFile(workspace_),relative);auto parent=QUrl::fromLocalFile(QFileInfo(target.toLocalFile()).absolutePath());auto created=storage::provider(parent).create(parent,QFileInfo(target.toLocalFile()).fileName(),false);refreshWorkspace();open(created);return true;}catch(const std::exception& e){lastError_=e.what();log(lastError_);return false;}}
bool Studio::createFolder(const QString& relative){try{auto target=storage::workspaceChild(QUrl::fromLocalFile(workspace_),relative);auto parent=QUrl::fromLocalFile(QFileInfo(target.toLocalFile()).absolutePath());storage::provider(parent).create(parent,QFileInfo(target.toLocalFile()).fileName(),true);refreshWorkspace();return true;}catch(const std::exception& e){lastError_=e.what();log(lastError_);return false;}}
bool Studio::renameFile(const QString& relative,const QString& name){try{auto url=storage::workspaceChild(QUrl::fromLocalFile(workspace_),relative);auto old=url.toLocalFile();auto renamed=storage::provider(url).rename(url,name).toLocalFile();for(auto* d:docs_)if(d->path()==old||d->path().startsWith(old+'/')){d->path_=renamed+d->path().mid(old.size());emit d->changed();}refreshWorkspace();saveSession();return true;}catch(const std::exception& e){lastError_=e.what();log(lastError_);return false;}}
bool Studio::trashFile(const QString& relative){try{auto url=storage::workspaceChild(QUrl::fromLocalFile(workspace_),relative);auto path=url.toLocalFile();for(auto* d:docs_)if((d->path()==path||d->path().startsWith(path+'/'))&&d->dirty())throw std::runtime_error("Save or close modified documents before deleting");auto dir=workspace_+"/.nandstudio-trash/"+QUuid::createUuid().toString(QUuid::WithoutBraces);if(!QDir().mkpath(dir))throw std::runtime_error("Cannot create recovery trash");auto target=dir+"/"+QFileInfo(path).fileName();if(!QDir().rename(path,target))throw std::runtime_error("Could not move item to recovery trash");for(int i=int(docs_.size())-1;i>=0;--i)if(docs_[i]->path()==path||docs_[i]->path().startsWith(path+'/'))closeDocumentDiscard(i);atomicSave(dir+"/original-path.txt",relative.toUtf8());log("Moved to recoverable trash: "+target);refreshWorkspace();return true;}catch(const std::exception& e){lastError_=e.what();log(lastError_);return false;}}
bool Studio::saveAll(){bool okay=true;for(auto* d:docs_)if(!d->save())okay=false;return okay;}
bool Studio::hasDirtyDocuments()const{for(auto* d:docs_)if(d->dirty())return true;return false;}
void Studio::closeDocumentDiscard(int i){if(i<0||i>=docs_.size())return;docs_[i]->dirty_=false;closeDocument(i);saveSession();}
void Studio::discardRecovery(){for(auto* d:docs_)d->dirty_=false;saveSession();}
void Studio::suspend(){key(0);cancel();saveSession();}
void Studio::setAutosaveSeconds(int seconds){autosaveTimer_.stop();if(seconds>0)autosaveTimer_.start(std::clamp(seconds,5,600)*1000);}
void Studio::navigateTo(const QString& path,int line,int column){open(QUrl::fromLocalFile(path));if(current()&&QFileInfo(current()->path())==QFileInfo(path))emit diagnostic(active_,line,column,QString());}
void Studio::build(bool vmFolder,bool bootstrap){
    auto* d=current();if(!d||busy_)return;auto ext=QFileInfo(d->path()).suffix();if(ext=="hdl"){loadHardware();return;}if(ext!="asm"&&ext!="jack"&&ext!="vm"){log("Build supports .asm, .jack, .vm and .hdl. Choose the appropriate test action for .tst files.");return;}
    // Saving is explicit in the UI; build consumes the exact visible immutable buffer.
    auto input=d->text().toUtf8();auto path=d->path();auto dest=QFileInfo(path).absolutePath()+"/"+QFileInfo(path).completeBaseName()+(ext=="asm"?".hack":ext=="vm"?".asm":".vm");
    std::map<std::string,std::string> vmFiles;
    if(vmFolder){
        if(ext!="vm"){log("Open a VM file before translating its folder.");return;}
        try{QDirIterator it(QFileInfo(path).absolutePath(),{"*.vm"},QDir::Files);while(it.hasNext()){const auto file=it.next();vmFiles[QFileInfo(file).completeBaseName().toStdString()]=read(file).toStdString();}}
        catch(const std::exception& e){log(e.what());return;}
        for(auto* doc:docs_)if(QFileInfo(doc->path()).absolutePath()==QFileInfo(path).absolutePath()&&QFileInfo(doc->path()).suffix()=="vm")vmFiles[QFileInfo(doc->path()).completeBaseName().toStdString()]=doc->text().toStdString();
        dest=QFileInfo(path).absolutePath()+"/"+QFileInfo(QFileInfo(path).absolutePath()).fileName()+".asm";
    }
    for(auto* doc:docs_)if(doc->path()==dest&&doc->dirty()){log("Generated output has unsaved edits; save or close it first");return;}
    QByteArray previous;bool existed=QFileInfo::exists(dest);try{if(existed)previous=read(dest);}catch(const std::exception& e){log(e.what());return;}
    busy_=true;log("Building visible buffer snapshot: "+path);emit stateChanged();
    task_.setFuture(QtConcurrent::run([input,path,dest,ext,previous,existed,vmFolder,vmFiles,bootstrap]{TaskResult r;r.path=path;try{auto s=input.toStdString();auto output=ext=="asm"?nand::machineText(nand::assemble(s).words):ext=="vm"?nand::translateVm(vmFolder?vmFiles:std::map<std::string,std::string>{{QFileInfo(path).completeBaseName().toStdString(),s}},bootstrap):nand::compileJack(s);QByteArray bytes=QByteArray::fromStdString(output);
#ifdef Q_OS_WIN
        bytes.replace("\n","\r\n");
#endif
        if(QFileInfo::exists(dest)!=existed||(existed&&read(dest)!=previous))throw std::runtime_error("Generated file changed externally; output was not written");
        if(!atomicSave(dest,bytes))throw std::runtime_error("Cannot save generated output");r.message="Build succeeded";r.artifact=dest;
    }catch(const nand::Error& e){r.error=true;r.line=e.line;r.column=e.column;r.message=path+":"+QString::number(e.line)+":"+QString::number(e.column)+": "+e.what();}catch(const std::exception& e){r.error=true;r.message=e.what();}return r;}));
}
void Studio::loadCpu(){
    if(!current()||busy_)return;
    try{
        auto text=current()->text().toStdString();const bool assembly=QFileInfo(current()->path()).suffix()=="asm";
        nand::Assembly compiled;if(assembly)compiled=nand::assemble(text);else compiled.words=nand::parseHack(text);
        cpu_.load(compiled.words);cpuSourceLines_=std::move(compiled.sourceLines);
        cpuSourcePath_=assembly?current()->path():QString();cpuSourceText_=current()->text();pauseReason_.clear();
        vmMode_=false;hardwareMode_=false;log("CPU loaded from visible buffer snapshot");emit stateChanged();
    }catch(const nand::Error& e){log(e.what());emit diagnostic(active_,e.line,e.column,e.what());}
}
void Studio::loadVm(){if(!current()||busy_)return;try{std::map<std::string,std::string> files;auto dir=QFileInfo(current()->path()).absolutePath();QDirIterator it(dir,{"*.vm"},QDir::Files);while(it.hasNext()){auto path=it.next();auto text=read(path).toStdString();for(auto* d:docs_)if(d->path()==path)text=d->text().toStdString();files[QFileInfo(path).completeBaseName().toStdString()]=text;}nand::Vm next;next.load(files);vm_=std::move(next);vmMode_=true;hardwareMode_=false;log("VM loaded from folder snapshot including open buffers. OS calls require .vm implementations in this folder.");emit stateChanged();}catch(const std::exception& e){log(e.what());}}
void Studio::step(int count){if(busy_||count<1)return;count=std::min(count,1000000);cancelled_=false;busy_=true;emit stateChanged();auto snapshot=this->snapshot();bool mode=vmMode_;bool hardwareMode=hardwareMode_;
    const auto breakpoints=mode?vmBreakpoints_:cpuBreakpoints_;
    execution_.setFuture(QtConcurrent::run([this,snapshot,mode,hardwareMode,count,breakpoints]{try{for(int n=0;n<count&&!cancelled_;++n){
        const auto pc=mode?int(snapshot->vm.pc):int(snapshot->cpu.pc);
        if(count>1&&!hardwareMode&&breakpoints.contains(pc)){snapshot->pauseReason="Breakpoint at PC "+QString::number(pc)+". Step once to continue past it.";break;}
        (mode?snapshot->vm.ram:snapshot->cpu.ram)[24576]=nand::Word(keyboard_.load());if(hardwareMode){snapshot->hardware.keyboard(nand::Word(keyboard_.load()));if(!snapshot->hardware.clockUp())snapshot->hardware.tick();snapshot->hardware.tock();}else if(mode)snapshot->vm.step();else snapshot->cpu.step();}}catch(const std::exception& e){snapshot->error=e.what();}return snapshot;}));
}
void Studio::reset(){if(busy_)return;if(hardwareMode_){log("To reset hardware, explicitly reload the HDL snapshot using Load HDL.");return;}if(vmMode_){vm_.pc=vm_.functions.contains("Sys.init")?vm_.functions.at("Sys.init"):0;vm_.time=0;}else cpu_.restart();emit stateChanged();}
void Studio::cancel(){cancelled_=true;}
void Studio::setMemory(int a,int v){if(busy_||a<0||a>=32768)return;(vmMode_?vm_.ram:cpu_.ram)[std::size_t(a)]=nand::Word(v);emit stateChanged();}
int Studio::memory(int a)const{return a<0||a>=32768?0:nand::signedWord((vmMode_?vm_.ram:cpu_.ram)[std::size_t(a)]);}
QVariantMap Studio::vmInspection()const{
    if(!vmMode_)return {};
    QVariantList stack,frames,segments;
    const auto sp=int(vm_.ram[0]);
    if(sp<=int(vm_.ram.size()))for(int address=std::max(256,sp-16);address<sp;++address)stack.append(QVariantMap{{"address",address},{"value",nand::signedWord(vm_.ram[std::size_t(address)])}});
    for(const auto& function:vm_.callStack)frames.append(QString::fromStdString(function));
    for(auto pair:{qMakePair("SP",0),qMakePair("LCL",1),qMakePair("ARG",2),qMakePair("THIS",3),qMakePair("THAT",4)})segments.append(QVariantMap{{"name",pair.first},{"value",int(vm_.ram[std::size_t(pair.second)])}});
    QVariantMap result{{"stack",stack},{"calls",frames},{"segments",segments},{"validSP",sp<=int(vm_.ram.size())},{"function",QString()},{"instruction",QString("Program counter outside loaded program")}};
    if(vm_.pc<vm_.code.size()){
        const auto& instruction=vm_.code[vm_.pc];QString command=QString::fromStdString(instruction.op);
        if(!instruction.arg.empty())command+=" "+QString::fromStdString(instruction.arg);
        if(instruction.op=="push"||instruction.op=="pop"||instruction.op=="function"||instruction.op=="call")command+=" "+QString::number(instruction.index);
        result["instruction"]=command;result["function"]=QString::fromStdString(instruction.scope);result["source"]=QString::fromStdString(instruction.file)+".vm:"+QString::number(instruction.line);
    }
    return result;
}
void Studio::key(int value){keyboard_=value;if(!busy_){hardware_.keyboard(nand::Word(value));(vmMode_?vm_.ram:cpu_.ram)[24576]=nand::Word(value);emit stateChanged();}}
QVariantMap Studio::state()const{
    QVariantList pins,parts;for(auto& p:hardware_.pins())pins.append(QVariantMap{{"name",QString::fromStdString(p.name)},{"direction",QString::fromStdString(p.direction)},{"width",p.width},{"value",nand::signedWord(p.value)}});
    for(auto& p:hardware_.components()){QVariantList partPins;for(auto& pin:p.pins)partPins.append(QVariantMap{{"name",QString::fromStdString(pin.name)},{"direction",QString::fromStdString(pin.direction)},{"value",nand::signedWord(pin.value)}});parts.append(QVariantMap{{"path",QString::fromStdString(p.path)},{"chip",QString::fromStdString(p.chip)},{"implementation",QString::fromStdString(p.implementation)},{"words",qulonglong(p.words)},{"pins",partPins}});}
    QVariantList hierarchy;for(const auto& item:hardware_.hierarchy())hierarchy.append(QVariantMap{{"path",QString::fromStdString(item.path)},{"chip",QString::fromStdString(item.chip)},{"builtin",item.builtin}});
    QVariantList points;auto sorted=(vmMode_?vmBreakpoints_:cpuBreakpoints_).values();std::sort(sorted.begin(),sorted.end());for(int point:sorted)points.append(point);
    const auto instruction=cpu_.rom[cpu_.pc&32767];
    return {{"breakpoints",points},{"pauseReason",pauseReason_},{"instructionBits",QString::number(instruction,2).rightJustified(16,'0')},{"hierarchy",hierarchy},{"mode",hardwareMode_?"Hardware Simulator":vmMode_?"VM Emulator":"CPU Emulator"},{"hardware",hardwareMode_},{"chip",QString::fromStdString(hardware_.name())},{"pins",pins},{"components",parts},{"clockUp",hardware_.clockUp()},{"hardwareTime",QString::fromStdString(hardware_.getText("time"))},{"A",nand::signedWord(cpu_.a)},{"D",nand::signedWord(cpu_.d)},{"PC",vmMode_?int(vm_.pc):int(cpu_.pc)},{"SP",vm_.ram[0]},{"time",qulonglong(vmMode_?vm_.time:cpu_.time)}};
}
QImage Studio::screen()const{QImage image(512,256,QImage::Format_RGB32);const auto& ram=vmMode_?vm_.ram:cpu_.ram;auto hardwareScreen=hardwareMode_?hardware_.screen():std::vector<nand::Word>{};for(int y=0;y<256;++y){auto* row=reinterpret_cast<QRgb*>(image.scanLine(y));for(int x=0;x<512;++x){int offset=y*32+x/16;auto word=hardwareMode_?(hardwareScreen.empty()?0:hardwareScreen[offset]):ram[16384+offset];row[x]=(word&(1u<<(x%16)))?qRgb(23,29,38):qRgb(234,241,225);}}return image;}
void Studio::test(bool vm){runTest(vm?nand::ScriptTool::Vm:nand::ScriptTool::Cpu);}
void Studio::testHardware(){runTest(nand::ScriptTool::Hardware);}
void Studio::runTest(nand::ScriptTool tool){if(!current()||busy_)return;for(auto* d:docs_)if(d->dirty()){log("Test scripts use disk files. Save all modified documents before testing.");return;}auto path=current()->path();if(!path.endsWith(".tst")){log("Open a .tst file first");return;}busy_=true;cancelled_=false;emit stateChanged();task_.setFuture(QtConcurrent::run([this,path,tool]{TaskResult r;r.path=path;try{
#ifdef Q_OS_WIN
    auto p=std::filesystem::path(path.toStdWString());
#else
    auto p=std::filesystem::path(path.toStdString());
#endif
    auto result=nand::runScript(p,tool,10000000,[this]{return cancelled_.load();});r.message=QString::fromStdString(result.message+"\n"+result.output);r.error=!result.passed;}catch(const std::exception& e){r.error=true;r.message=e.what();}return r;}));}
std::shared_ptr<ExecutionResult> Studio::snapshot()const{auto r=std::make_shared<ExecutionResult>();r->cpu=cpu_;r->vm=vm_;r->hardware=hardware_;r->vmMode=vmMode_;r->hardwareMode=hardwareMode_;r->hardwarePath=hardwarePath_;r->hardwareSources=hardwareSources_;r->hardwareMessage=hardwareMessage_;return r;}
bool Studio::hardwareNeedsReload()const{
    if(current()&&current()->path().endsWith(".hdl")&&(!hardwareMode_||current()->path()!=hardwarePath_))return true;
    if(!hardwareMode_)return false;
    for(auto* d:docs_)if(d->path().endsWith(".hdl")&&QFileInfo(d->path()).absolutePath()==QFileInfo(hardwarePath_).absolutePath()&&hardwareSources_.value(d->path())!=d->text())return true;
    return false;
}
QMap<QString,QString> Studio::collectHardwareSources(const QString& target)const{
    QMap<QString,QString> sources;const auto dir=QFileInfo(target).absolutePath();
    QDirIterator it(dir,{"*.hdl"},QDir::Files);
    while(it.hasNext()){
        const auto path=it.next();auto text=QString::fromUtf8(read(path));
        if(text.startsWith(QChar(0xfeff)))text.remove(0,1);
        text.replace("\r\n","\n");text.replace('\r','\n');sources[path]=text;
    }
    // Visible editor snapshots remain authoritative; closed dependencies use disk.
    for(auto* d:docs_)if(QFileInfo(d->path()).absolutePath()==dir&&d->path().endsWith(".hdl"))sources[d->path()]=d->text();
    return sources;
}
namespace {void applyPinEdits(nand::Hardware&,const QVariantMap&);}
void Studio::loadHardware(){beginHardwareLoad(false,{});}
void Studio::evaluateHardwareWithInputs(const QVariantMap& inputs){
    if(busy_)return;
    bool reload=hardwareNeedsReload();
    // Disk reads belong to the explicit action, not frequently evaluated QML bindings.
    if(!reload&&hardwareMode_)try{reload=collectHardwareSources(hardwarePath_)!=hardwareSources_;}catch(const std::exception&){reload=true;}
    if(reload)beginHardwareLoad(true,inputs);
    else if(hardwareMode_)hardwareActionWithInputs("eval",inputs);
    else{hardwareMessage_="Open an HDL file and choose Load & Eval HDL first.";log(hardwareMessage_);emit stateChanged();}
}
void Studio::beginHardwareLoad(bool evaluate,const QVariantMap& inputs){
    if(busy_)return;
    const auto target=current()&&current()->path().endsWith(".hdl")?current()->path():hardwareMode_?hardwarePath_:QString();
    if(target.isEmpty()){log("Open an .hdl file first");return;}
    // The resolver owns immutable text for every project-local dependency.
    try{std::map<std::string,std::string> files;auto sources=collectHardwareSources(target);auto dir=QFileInfo(target).absolutePath();for(auto i=sources.cbegin();i!=sources.cend();++i)files[QFileInfo(i.key()).completeBaseName().toStdString()]=i.value().toStdString();
        hardwareEvent_=evaluate?"load-eval":"load";auto path=target;auto chip=QFileInfo(path).completeBaseName().toStdString();auto next=snapshot();cancelled_=false;busy_=true;emit stateChanged();log("Loading HDL folder snapshot including visible buffers");
        execution_.setFuture(QtConcurrent::run([this,next,files=std::move(files),sources,path,dir,chip,evaluate,inputs]{try{
            QVariantMap previousInputs;
            if(evaluate&&next->hardwareMode&&next->hardwarePath==path)
                for(const auto& pin:next->hardware.pins())if(pin.direction=="input")previousInputs[QString::fromStdString(pin.name)]=int(pin.value);
            next->hardware.load(chip,[&](const std::string& n)->std::optional<std::string>{auto i=files.find(n);if(i==files.end())return std::nullopt;return i->second;},[this]{return cancelled_.load();});
            if(evaluate){QVariantMap valid;for(auto& pin:next->hardware.pins())if(pin.direction=="input"){
                const auto name=QString::fromStdString(pin.name);
                if(inputs.contains(name))valid[name]=inputs[name];
                else if(previousInputs.contains(name))valid[name]=previousInputs[name];
            }applyPinEdits(next->hardware,valid);next->hardware.eval();}
            next->hardwareMode=true;next->vmMode=false;next->hardwarePath=path;next->hardwareSources=sources;
            next->hardwareMessage=evaluate?"Loaded and evaluated visible HDL snapshot.":"Loaded visible HDL snapshot.";
            const auto hierarchy=next->hardware.hierarchy();QStringList empty;
            for(const auto& item:hierarchy)if(!item.builtin){bool child=false;for(const auto& other:hierarchy)if(other.path.starts_with(item.path+"/")){child=true;break;}if(!child&&!empty.contains(QString::fromStdString(item.chip)))empty.append(QString::fromStdString(item.chip));}
            if(!empty.isEmpty())next->hardwareMessage+=" Warning: "+empty.join(", ")+" has an empty PARTS section. Project-local chips override built-ins; unfinished dependencies can keep outputs at zero.";
        }catch(const nand::Error& e){next->error=QString::fromStdString(e.file)+":"+QString::number(e.line)+":"+QString::number(e.column)+": "+e.what();next->errorPath=e.file.empty()?path:QDir(dir).filePath(QString::fromStdString(e.file));next->errorLine=e.line;next->errorColumn=e.column;}catch(const std::exception& e){next->error=e.what();next->errorPath=path;}return next;}));
    }catch(const std::exception& e){hardwareMessage_=QString::fromUtf8(e.what());hardwareEvaluation_="HDL load failed: "+hardwareMessage_;log(hardwareEvaluation_);emit stateChanged();}
}
namespace {
void applyPinEdits(nand::Hardware& hardware,const QVariantMap& inputs){
    for(auto it=inputs.cbegin();it!=inputs.cend();++it){
        auto text=it.value().toString().trimmed();bool okay=false;
        int value=text.toInt(&okay,10);
        if(!okay)throw nand::Error("Invalid decimal value for pin "+it.key().toStdString()+": "+text.toStdString());
        hardware.set(it.key().toStdString(),value);
    }
}
}
bool Studio::commitHardwareInputs(const QVariantMap& inputs){
    if(busy_||!hardwareMode_)return false;
    try{auto next=hardware_;applyPinEdits(next,inputs);hardware_=std::move(next);emit stateChanged();return true;}
    catch(const std::exception& e){log(QString("Hardware input error: ")+e.what());return false;}
}
void Studio::hardwareAction(const QString& action){if(action=="eval")evaluateHardwareWithInputs({});else hardwareActionWithInputs(action,{});}
void Studio::hardwareActionWithInputs(const QString& action,const QVariantMap& inputs){
    if(busy_||!hardwareMode_)return;
    auto next=snapshot();
    try{applyPinEdits(next->hardware,inputs);}catch(const std::exception& e){hardwareEvaluation_=QString("Hardware input error: ")+e.what();log(hardwareEvaluation_);emit stateChanged();return;}
    hardwareEvent_=action;busy_=true;emit stateChanged();
    execution_.setFuture(QtConcurrent::run([next,action]{try{
        if(action=="eval")next->hardware.eval();
        else if(action=="tick")next->hardware.tick();
        else if(action=="tock")next->hardware.tock();
        else if(action=="cycle"){if(!next->hardware.clockUp())next->hardware.tick();next->hardware.tock();}
        else throw nand::Error("Unknown hardware action");
    }catch(const std::exception& e){next->error=e.what();}return next;}));
}
void Studio::setHardware(const QString& variable,int value){if(busy_)return;try{hardware_.set(variable.toStdString(),value);emit stateChanged();}catch(const std::exception& e){log(e.what());}}
QString Studio::hardwareValue(const QString& variable)const{try{return QString::fromStdString(hardware_.getText(variable.toStdString()));}catch(const std::exception& e){return QString::fromUtf8(e.what());}}
void Studio::loadHardwareRom(const QString& component){if(busy_||!current())return;try{auto text=current()->text().toStdString();auto program=QFileInfo(current()->path()).suffix()=="asm"?nand::assemble(text).words:nand::parseHack(text);hardware_.loadRom(component.toStdString(),program);log("ROM loaded from visible buffer snapshot");emit stateChanged();}catch(const std::exception& e){log(e.what());}}
void Studio::command(const QString& text){
    auto args=text.simplified().split(' ');
    if(args[0]=="translate-vm")build(true,args.contains("--bootstrap"));
    else if(args[0]=="preview")previewConversion(args.contains("--bootstrap"));
    else if(args[0]=="build")build();
    else if(args[0]=="step")step(args.size()>1?args[1].toInt():1);
    else if(args[0]=="load-cpu")loadCpu();else if(args[0]=="load-vm")loadVm();else if(args[0]=="load-hdl")loadHardware();
    else if(args[0]=="test-hdl")testHardware();
    else if(args[0]=="eval"||args[0]=="tick"||args[0]=="tock")hardwareAction(args[0]);
    else if(args[0]=="set-pin"&&args.size()==3)setHardware(args[1],args[2].toInt());
    else if(args[0]=="test-cpu")test(false);else if(args[0]=="test-vm")test(true);
    else if(args[0]=="reset")reset();else if(args[0]=="stop")cancel();
    else log("Commands: build, preview [--bootstrap], translate-vm [--bootstrap], load-cpu, load-vm, load-hdl, eval, tick, tock, set-pin name value, test-hdl, step [count], reset, stop, test-cpu, test-vm");
}
void Studio::findInProject(const QString& query){if(query.isEmpty())return;auto files=files_;QMap<QString,QString> buffers;for(auto* d:docs_)buffers[d->path()]=d->text();searchTask_.setFuture(QtConcurrent::run([files,buffers,query]{QVariantList matches;for(auto f:files){auto entry=f.toMap();auto path=entry["path"].toUrl().toLocalFile();try{auto text=(buffers.contains(path)?buffers[path]:QString::fromUtf8(read(path))).split('\n');for(int i=0;i<text.size();++i){int column=int(text[i].indexOf(query,0,Qt::CaseInsensitive));if(column>=0){matches.append(QVariantMap{{"path",path},{"line",i+1},{"column",column+1},{"message",text[i]},{"label",entry["name"].toString()+":"+QString::number(i+1)+":"+QString::number(column+1)+"  "+text[i]}});if(matches.size()>=1000)return matches;}}}catch(...){}}return matches;}));}
bool Studio::closeDocument(int i){if(i<0||i>=docs_.size())return false;if(docs_[i]->dirty()){log("Save the document before closing; unsaved text is retained");return false;}auto* d=docs_.takeAt(i);d->deleteLater();active_=std::min(active_,int(docs_.size())-1);emit documentsChanged();emit activeChanged();return true;}
void Studio::saveSession(){QJsonArray array;for(auto* d:docs_){QJsonObject o{{"path",d->path()},{"dirty",d->dirty()}};if(d->dirty()){o["text"]=d->text();o["original"]=QString::fromLatin1(d->original.toBase64());}array.append(o);}QJsonObject root{{"workspace",workspace_},{"active",active_},{"documents",array}};if(!atomicSave(sessionPath(),QJsonDocument(root).toJson()))log("Recovery save failed");}
void Studio::recover(){if(!QFileInfo::exists(sessionPath()))return;try{auto root=QJsonDocument::fromJson(read(sessionPath())).object();auto w=root["workspace"].toString();if(QFileInfo(w).isDir())openWorkspace(QUrl::fromLocalFile(w));for(auto value:root["documents"].toArray()){auto o=value.toObject();open(QUrl::fromLocalFile(o["path"].toString()));if(current()&&current()->path()==o["path"].toString()&&o["dirty"].toBool()){current()->setText(o["text"].toString());current()->original=QByteArray::fromBase64(o["original"].toString().toLatin1());log("Recovered unsaved buffer: "+current()->path());}}setActive(root["active"].toInt());log("Session restored. Simulation is stopped; reload explicitly to execute.");}catch(const std::exception& e){log(e.what());}}

void Studio::recordHardware(const QString& event){
    QVariantMap values;
    for(const auto& pin:hardware_.pins())values[QString::fromStdString(pin.name)]=nand::signedWord(pin.value);
    hardwareTrace_.append(QVariantMap{{"event",event},{"time",QString::fromStdString(hardware_.getText("time"))},{"clock",hardware_.clockUp()},{"values",values}});
    if(hardwareTrace_.size()>256)hardwareTrace_.removeFirst();
}
void Studio::clearHardwareTrace(){hardwareTrace_.clear();if(hardwareMode_)recordHardware("capture");emit stateChanged();}
QString Studio::formatWord(int value,int radix)const{
    if(radix==2)return QString::number(nand::Word(value),2).rightJustified(16,'0');
    if(radix==16)return QString::number(nand::Word(value),16).rightJustified(4,'0').toUpper();
    return QString::number(nand::signedWord(nand::Word(value)));
}

QVariantMap Studio::hardwareDiagram(const QString& selected)const{
    QString path=selected.isEmpty()?QString::fromStdString(hardware_.name()):selected;
    QVariantList blocks,connections;QMap<QString,QPointF> endpoints;int nextY=12;
    for(const auto& instance:hardware_.hierarchy()){
        QString name=QString::fromStdString(instance.path);
        bool parent=name==path;
        if(!parent&&name.section('/',0,-2)!=path)continue;
        int x=parent?12:330,y=parent?12:nextY,height=40+int(instance.pins.size())*24;
        QVariantList pins;int row=0;
        for(const auto& pin:instance.pins){
            QString key=name+"."+QString::fromStdString(pin.name);
            int py=y+42+row++*24;int px=parent?252:330;
            endpoints[key]=QPointF(px,py);
            pins.append(QVariantMap{{"name",QString::fromStdString(pin.name)},{"direction",QString::fromStdString(pin.direction)},{"width",pin.width},{"value",nand::signedWord(pin.value)},{"y",py}});
        }
        blocks.append(QVariantMap{{"path",name},{"name",parent?name:name.section('/',-1)},{"builtin",instance.builtin},{"x",x},{"y",y},{"width",240},{"height",height},{"pins",pins}});
        if(!parent)nextY+=height+24;
    }
    int diagramHeight=120;
    for(const auto& value:blocks){auto block=value.toMap();diagramHeight=std::max(diagramHeight,block["y"].toInt()+block["height"].toInt()+16);}
    for(const auto& wire:hardware_.wires()){
        QString source=QString::fromStdString(wire.source),target=QString::fromStdString(wire.target);
        if(!endpoints.contains(source)||!endpoints.contains(target))continue;
        auto from=endpoints[source],to=endpoints[target];
        connections.append(QVariantMap{{"source",source},{"target",target},{"x1",from.x()},{"y1",from.y()},{"x2",to.x()},{"y2",to.y()},{"sourceLo",wire.sourceLo},{"targetLo",wire.targetLo},{"width",wire.width},{"value",nand::signedWord(wire.value)}});
    }
    return {{"blocks",blocks},{"wires",connections},{"width",590},{"height",diagramHeight}};
}
