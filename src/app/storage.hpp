// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include <QByteArray>
#include <QUrl>
#include <QStringList>
#include <memory>
namespace storage {
struct Entry {QUrl url;QString name;bool directory=false;};
struct Capabilities {bool read=true,write=false,create=false,rename=false,remove=false,atomicReplace=false;};
class Provider {
public:
    virtual ~Provider()=default;
    virtual Capabilities capabilities(const QUrl&)const=0;
    virtual QByteArray read(const QUrl&)const=0;
    virtual void write(const QUrl&,const QByteArray&)const=0;
    virtual QList<Entry> list(const QUrl&)const=0;
    virtual QUrl child(const QUrl&,const QString&)const=0;
    virtual QUrl create(const QUrl&,const QString&,bool directory)const=0;
    virtual QUrl rename(const QUrl&,const QString&)const=0;
};
const Provider& provider(const QUrl&);
// Resolves through existing ancestors and rejects workspace escape/symlinks.
QUrl workspaceChild(const QUrl& root,const QString& relative);
}
