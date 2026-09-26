// SPDX-License-Identifier: GPL-3.0-or-later
#include "storage.hpp"
#include <QFile>
#include <QFileInfo>
#include <QSaveFile>
#include <QDir>
#include <QRegularExpression>
#include <QSet>
#include <QDirIterator>
#include <stdexcept>
namespace storage { namespace {
[[noreturn]]void fail(const QString& message){throw std::runtime_error(message.toStdString());}
QString local(const QUrl& url){if(!url.isLocalFile())fail("A local file URL is required by this provider");return url.toLocalFile();}
void validateName(const QString& name){if(name.isEmpty()||name=="."||name==".."||name.contains('/')||name.contains('\\')||name.contains(QChar(0))||name.contains(':'))fail("Invalid file or folder name");
#ifdef Q_OS_WIN
    if(name.endsWith('.')||name.endsWith(' ')||name.contains(QRegularExpression("[<>\"|?*]")))fail("Invalid Windows filename");
    auto stem=name.section('.',0,0).toUpper();if(QStringList{"CON","PRN","AUX","NUL","COM1","COM2","COM3","COM4","COM5","COM6","COM7","COM8","COM9","LPT1","LPT2","LPT3","LPT4","LPT5","LPT6","LPT7","LPT8","LPT9"}.contains(stem))fail("Reserved Windows filename");
#endif
}
class FileProvider final:public Provider {
public:
    Capabilities capabilities(const QUrl& url)const override{auto p=QFileInfo(local(url));return {p.isReadable(),p.isWritable(),p.isDir()&&p.isWritable(),p.isWritable(),p.isWritable(),true};}
    QByteArray read(const QUrl& url)const override{QFile file(local(url));if(!file.open(QIODevice::ReadOnly))fail(file.errorString());auto bytes=file.read(32*1024*1024+1);if(bytes.size()>32*1024*1024)fail("File exceeds 32 MiB limit");if(file.error()!=QFileDevice::NoError)fail(file.errorString());return bytes;}
    void write(const QUrl& url,const QByteArray& data)const override{QSaveFile file(local(url));if(!file.open(QIODevice::WriteOnly)||file.write(data)!=data.size()||!file.commit())fail(file.errorString());}
    QList<Entry> list(const QUrl& url)const override{QDir dir(local(url));if(!dir.exists())fail("Workspace folder is missing");QList<Entry> entries;for(auto& f:dir.entryInfoList(QDir::AllEntries|QDir::Hidden|QDir::NoDotAndDotDot|QDir::NoSymLinks,QDir::DirsFirst|QDir::Name))entries.append({QUrl::fromLocalFile(f.absoluteFilePath()),f.fileName(),f.isDir()});return entries;}
    QUrl child(const QUrl& url,const QString& name)const override{validateName(name);return QUrl::fromLocalFile(QDir(local(url)).filePath(name));}
    QUrl create(const QUrl& dir,const QString& name,bool folder)const override{auto url=child(dir,name);auto path=local(url);if(QFileInfo::exists(path))fail("A file or folder with this name already exists");if(folder){if(!QDir().mkdir(path))fail("Cannot create folder");}else{QFile file(path);if(!file.open(QIODevice::WriteOnly|QIODevice::NewOnly))fail(file.errorString());}return url;}
    QUrl rename(const QUrl& url,const QString& name)const override{auto target=child(QUrl::fromLocalFile(QFileInfo(local(url)).absolutePath()),name);if(QFileInfo::exists(local(target)))fail("Destination already exists");if(!QDir().rename(local(url),local(target)))fail("Rename failed");return target;}
};
#ifdef Q_OS_ANDROID
// Qt 6.11's Android content file engine resolves these URIs through
// DocumentsContract. They are never converted to ordinary filesystem paths.
class ContentProvider final:public Provider {
    static QString uri(const QUrl& url){return url.toString(QUrl::FullyEncoded);}
public:
    Capabilities capabilities(const QUrl& url)const override{QFileInfo f(uri(url));return {f.isReadable(),f.isWritable(),false,false,false,false};}
    QByteArray read(const QUrl& url)const override{
        QFile f(uri(url));if(!f.open(QIODevice::ReadOnly))fail("Cannot read document; select the folder again to renew access: "+f.errorString());
        auto bytes=f.read(32*1024*1024+1);if(bytes.size()>32*1024*1024)fail("File exceeds 32 MiB limit");if(f.error()!=QFileDevice::NoError)fail(f.errorString());return bytes;
    }
    void write(const QUrl& url,const QByteArray& bytes)const override{
        QFile f(uri(url));if(!f.open(QIODevice::WriteOnly|QIODevice::Truncate)||f.write(bytes)!=bytes.size()||!f.flush())fail("Document-provider write failed: "+f.errorString());
    }
    QList<Entry> list(const QUrl& url)const override{
        QFileInfo root(uri(url));if(!root.isDir()||!root.isReadable())fail("Folder access unavailable; select it again in Android's chooser");
        QList<Entry> result;QDir dir(uri(url));for(const auto& f:dir.entryInfoList(QDir::AllEntries|QDir::Hidden|QDir::NoDotAndDotDot,QDir::DirsFirst|QDir::Name))result.append({QUrl(f.filePath()),f.fileName(),f.isDir()});return result;
    }
    QUrl child(const QUrl& url,const QString& name)const override{validateName(name);return QUrl(uri(url)+"/"+QString::fromLatin1(QUrl::toPercentEncoding(name)));}
    QUrl create(const QUrl& parent,const QString& name,bool folder)const override{
        validateName(name);for(const auto& entry:list(parent))if(entry.name==name)fail("Destination already exists");
        // QDir/QFile pass the new child name to Android's DocumentsContract.
        auto path=QDir(uri(parent)).filePath(name);
        if(folder){if(!QDir(uri(parent)).mkdir(name))fail("Provider cannot create a folder here");}
        else{QFile f(path);if(!f.open(QIODevice::WriteOnly|QIODevice::NewOnly))fail("Provider cannot create document: "+f.errorString());}
        for(const auto& entry:list(parent))if(entry.name==name)return entry.url;
        fail("Provider did not return the created document");
    }
    QUrl rename(const QUrl&,const QString&)const override{fail("Direct provider rename is not supported; edit the imported local copy");}
};
#endif
}
QUrl copyCourseWorkspace(const QUrl& destinationParent,const QString& newName,const std::function<bool()>& cancelled){
    validateName(newName);
    auto destination=provider(destinationParent).create(destinationParent,newName,true);
    try{
        QDirIterator files(":/starters",QDir::Files|QDir::Hidden,QDirIterator::Subdirectories);
        int count=0;
        while(files.hasNext()){
            if(cancelled&&cancelled())fail("Course copy cancelled");
            const auto path=files.next();auto parts=path.mid(QString(":/starters/").size()).split('/');
            auto parent=destination;
            for(int i=0;i+1<parts.size();++i){
                bool found=false;
                for(const auto& entry:provider(parent).list(parent))if(entry.name==parts[i]){if(!entry.directory)fail("Course folder collision");parent=entry.url;found=true;break;}
                if(!found)parent=provider(parent).create(parent,parts[i],true);
            }
            QFile file(path);if(!file.open(QIODevice::ReadOnly))fail(file.errorString());
            auto target=provider(parent).create(parent,parts.last(),false);
            provider(target).write(target,file.readAll());++count;
        }
        if(count==0)fail("Bundled course resources are missing");
    }catch(const std::exception& error){fail(QString::fromUtf8(error.what())+". Incomplete copy retained at "+destination.toString());}
    return destination;
}
const Provider& provider(const QUrl& url){static FileProvider files;if(url.isLocalFile())return files;
#ifdef Q_OS_ANDROID
    static ContentProvider documents;if(url.scheme()=="content")return documents;
#endif
    fail("No provider registered for "+url.scheme()+". Import this project into a local workspace before editing.");}
QUrl workspaceChild(const QUrl& root,const QString& relative){
    if(!root.isLocalFile()) {auto url=root;for(const auto& part:relative.split('/'))url=provider(url).child(url,part);return url;}
    if(QDir::isAbsolutePath(relative)||relative.contains('\\'))fail("Use a workspace-relative path with forward slashes");auto parts=relative.split('/');if(parts.isEmpty())fail("Filename required");for(auto& part:parts)validateName(part);
    auto base=QFileInfo(root.toLocalFile()).canonicalFilePath();auto path=QDir(root.toLocalFile()).filePath(relative);auto parent=QFileInfo(QFileInfo(path).absolutePath()).canonicalFilePath();if(base.isEmpty()||parent.isEmpty()||(parent!=base&&!parent.startsWith(base+'/')))fail("Path must stay within the workspace and its parent folder must exist");return QUrl::fromLocalFile(path);
}
QUrl copyWorkspace(const QUrl& source,const QUrl& destinationParent,const QString& newName,const std::function<bool()>& cancelled){
    validateName(newName);const auto& targetProvider=provider(destinationParent);
    // Reject copying a local folder into itself before creating anything.
    if(source.isLocalFile()&&destinationParent.isLocalFile()){
        auto from=QFileInfo(source.toLocalFile()).canonicalFilePath();auto to=QFileInfo(destinationParent.toLocalFile()).canonicalFilePath();
        if(from.isEmpty()||to.isEmpty()||to==from||to.startsWith(from+'/'))fail("Copy destination must be outside the source workspace");
    }
    if(cancelled&&cancelled())fail("Workspace copy cancelled");
    auto destination=targetProvider.create(destinationParent,newName,true);
    struct Pending{QUrl from,to;int depth;};QList<Pending> pending{{source,destination,0}};
    QSet<QString> visited;qsizetype entries=0;quint64 total=0;
    try{while(!pending.isEmpty()){
        auto next=pending.takeLast();if(next.depth>128)fail("Workspace depth exceeds 128");
        if(visited.contains(next.from.toString()))fail("Workspace contains a directory cycle");visited.insert(next.from.toString());
        if(next.from.isLocalFile())for(const auto& f:QDir(next.from.toLocalFile()).entryInfoList(QDir::AllEntries|QDir::Hidden|QDir::NoDotAndDotDot))if(f.isSymLink())fail("Workspace copy does not support symbolic links");
        QSet<QString> names;
        for(const auto& entry:provider(next.from).list(next.from)){
            if(cancelled&&cancelled())fail("Workspace copy cancelled");
            if(++entries>10000)fail("Workspace exceeds 10,000 entries");validateName(entry.name);
            if(entry.name==".nandstudio-trash"||entry.name==".git")continue;
            if(names.contains(entry.name.toCaseFolded()))fail("Workspace has colliding filenames");names.insert(entry.name.toCaseFolded());
            auto target=provider(next.to).create(next.to,entry.name,entry.directory);
            if(entry.directory)pending.append({entry.url,target,next.depth+1});
            else{auto data=provider(entry.url).read(entry.url);total+=quint64(data.size());if(total>256*1024*1024)fail("Workspace exceeds 256 MiB");provider(target).write(target,data);}
        }
    }}catch(const std::exception& error){fail(QString::fromUtf8(error.what())+". Incomplete copy retained at "+destination.toString()+"; it was not opened or reported as saved.");}
    return destination;
}
}
