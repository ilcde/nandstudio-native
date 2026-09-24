// SPDX-License-Identifier: GPL-3.0-or-later
#include "storage.hpp"
#include <QFile>
#include <QFileInfo>
#include <QSaveFile>
#include <QDir>
#include <QRegularExpression>
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
    QByteArray read(const QUrl& url)const override{QFile file(local(url));if(!file.open(QIODevice::ReadOnly))fail(file.errorString());if(file.size()>32*1024*1024)fail("File exceeds 32 MiB limit");return file.readAll();}
    void write(const QUrl& url,const QByteArray& data)const override{QSaveFile file(local(url));if(!file.open(QIODevice::WriteOnly)||file.write(data)!=data.size()||!file.commit())fail(file.errorString());}
    QList<Entry> list(const QUrl& url)const override{QDir dir(local(url));if(!dir.exists())fail("Workspace folder is missing");QList<Entry> entries;for(auto& f:dir.entryInfoList(QDir::AllEntries|QDir::NoDotAndDotDot|QDir::NoSymLinks,QDir::DirsFirst|QDir::Name))entries.append({QUrl::fromLocalFile(f.absoluteFilePath()),f.fileName(),f.isDir()});return entries;}
    QUrl child(const QUrl& url,const QString& name)const override{validateName(name);return QUrl::fromLocalFile(QDir(local(url)).filePath(name));}
    QUrl create(const QUrl& dir,const QString& name,bool folder)const override{auto url=child(dir,name);auto path=local(url);if(QFileInfo::exists(path))fail("A file or folder with this name already exists");if(folder){if(!QDir().mkdir(path))fail("Cannot create folder");}else{QFile file(path);if(!file.open(QIODevice::WriteOnly|QIODevice::NewOnly))fail(file.errorString());}return url;}
    QUrl rename(const QUrl& url,const QString& name)const override{auto target=child(QUrl::fromLocalFile(QFileInfo(local(url)).absolutePath()),name);if(QFileInfo::exists(local(target)))fail("Destination already exists");if(!QDir().rename(local(url),local(target)))fail("Rename failed");return target;}
};
}
const Provider& provider(const QUrl& url){static FileProvider files;if(url.isLocalFile())return files;fail("No provider registered for "+url.scheme()+". Import this project into a local workspace before editing.");}
QUrl workspaceChild(const QUrl& root,const QString& relative){
    if(!root.isLocalFile()) {auto url=root;for(const auto& part:relative.split('/'))url=provider(url).child(url,part);return url;}
    if(QDir::isAbsolutePath(relative)||relative.contains('\\'))fail("Use a workspace-relative path with forward slashes");auto parts=relative.split('/');if(parts.isEmpty())fail("Filename required");for(auto& part:parts)validateName(part);
    auto base=QFileInfo(root.toLocalFile()).canonicalFilePath();auto path=QDir(root.toLocalFile()).filePath(relative);auto parent=QFileInfo(QFileInfo(path).absolutePath()).canonicalFilePath();if(base.isEmpty()||parent.isEmpty()||(parent!=base&&!parent.startsWith(base+'/')))fail("Path must stay within the workspace and its parent folder must exist");return QUrl::fromLocalFile(path);
}
}
