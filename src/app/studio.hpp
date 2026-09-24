// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include "core.hpp"
#include "hdl.hpp"
#include <QObject>
#include <QVariantList>
#include <QQuickTextDocument>
#include <QFutureWatcher>
#include <QTimer>
#include <QImage>
#include <QQuickImageProvider>
#include <atomic>
#include <memory>
class Document : public QObject {
    Q_OBJECT
    Q_PROPERTY(QString path READ path NOTIFY changed)
    Q_PROPERTY(QString name READ name NOTIFY changed)
    Q_PROPERTY(QString text READ text WRITE setText NOTIFY textChanged)
    Q_PROPERTY(bool dirty READ dirty NOTIFY changed)
    Q_PROPERTY(QString diskText READ diskText NOTIFY changed)
public:
    explicit Document(QString path, QObject* parent=nullptr);
    QString path()const{return path_;}QString name()const;QString text()const{return text_;}
    bool dirty()const{return dirty_;}void setText(const QString& t);
    Q_INVOKABLE bool save();
    Q_INVOKABLE bool resolveConflict(const QString& action);
    bool hasConflict()const{return conflictPending_;}
    QString diskText()const{return QString::fromUtf8(conflictingBytes_);}
    Q_INVOKABLE void highlight(QQuickTextDocument* document);
    QByteArray original;QString path_,text_,newline="\n";bool dirty_=false,bom=false;
signals:void changed();void textChanged();void error(QString message);void conflict();
private:QByteArray conflictingBytes_;bool conflictPending_=false;
};
struct TaskResult {QString message,artifact,text,path;int line=0,column=0;bool error=false;};
struct WorkspaceResult {QString path,error;QVariantList files;};
struct ExecutionResult {nand::Cpu cpu;nand::Vm vm;nand::Hardware hardware;bool vmMode=false,hardwareMode=false;QString error;};
class Studio : public QObject {
    Q_OBJECT
    Q_PROPERTY(QVariantList documents READ documents NOTIFY documentsChanged)
    Q_PROPERTY(QVariantList files READ files NOTIFY filesChanged)
    Q_PROPERTY(QString workspace READ workspace NOTIFY filesChanged)
    Q_PROPERTY(QString output READ output NOTIFY outputChanged)
    Q_PROPERTY(bool busy READ busy NOTIFY stateChanged)
    Q_PROPERTY(QVariantMap state READ state NOTIFY stateChanged)
    Q_PROPERTY(int active READ active WRITE setActive NOTIFY activeChanged)
    Q_PROPERTY(QString lastError READ lastError NOTIFY outputChanged)
    Q_PROPERTY(QVariantList searchResults READ searchResults NOTIFY searchChanged)
    Q_PROPERTY(QVariantList diagnostics READ diagnostics NOTIFY diagnosticsChanged)
    Q_PROPERTY(QVariantList hardwareTrace READ hardwareTrace NOTIFY stateChanged)
public:
    Studio();~Studio()override;
    QVariantList documents()const;QVariantList files()const{return files_;}
    QString workspace()const{return workspace_;}QString output()const{return output_;}bool busy()const{return busy_;}
    QVariantMap state()const;int active()const{return active_;}void setActive(int a);
    QVariantList hardwareTrace()const{return hardwareTrace_;}
    Q_INVOKABLE void clearHardwareTrace();
    Q_INVOKABLE QString formatWord(int value,int radix=10)const;
    QString lastError()const{return lastError_;}QVariantList searchResults()const{return searchResults_;}QVariantList diagnostics()const{return diagnostics_;}
    Q_INVOKABLE void open(const QUrl& url);
    Q_INVOKABLE void openWorkspace(const QUrl& url);
    Q_INVOKABLE void openLocalWorkspace(const QString& path) { openWorkspace(QUrl::fromLocalFile(path)); }
    Q_INVOKABLE bool createFile(const QString& relative);
    Q_INVOKABLE bool createFolder(const QString& relative);
    Q_INVOKABLE bool renameFile(const QString& relative,const QString& name);
    Q_INVOKABLE bool trashFile(const QString& relative);
    Q_INVOKABLE void refreshWorkspace();
    Q_INVOKABLE bool saveAll();Q_INVOKABLE bool hasDirtyDocuments()const;
    Q_INVOKABLE void closeDocumentDiscard(int index);
    Q_INVOKABLE void discardRecovery();Q_INVOKABLE void suspend();
    Q_INVOKABLE void setAutosaveSeconds(int seconds);
    Q_INVOKABLE void navigateTo(const QString& path,int line,int column);
    Q_INVOKABLE void build();Q_INVOKABLE void loadCpu();Q_INVOKABLE void loadVm();
    Q_INVOKABLE void loadHardware();Q_INVOKABLE void hardwareAction(const QString& action);
    Q_INVOKABLE void hardwareActionWithInputs(const QString& action,const QVariantMap& inputs);
    Q_INVOKABLE bool commitHardwareInputs(const QVariantMap& inputs);
    Q_INVOKABLE void setHardware(const QString& variable,int value);
    Q_INVOKABLE QString hardwareValue(const QString& variable)const;
    Q_INVOKABLE void loadHardwareRom(const QString& component);
    Q_INVOKABLE void testHardware();
    Q_INVOKABLE void step(int count=1);Q_INVOKABLE void reset();Q_INVOKABLE void cancel();
    Q_INVOKABLE void setMemory(int address,int value);Q_INVOKABLE int memory(int address)const;
    Q_INVOKABLE void key(int value);Q_INVOKABLE void test(bool vm);
    Q_INVOKABLE void command(const QString& text);
    Q_INVOKABLE void findInProject(const QString& query);
    Q_INVOKABLE bool closeDocument(int index);
    QImage screen()const;
    void log(QString text);
signals:
    void documentsChanged();void filesChanged();void outputChanged();void stateChanged();void activeChanged();
    void diagnostic(int document,int line,int column,QString message);
    void conflict(Document* document);void searchChanged();void diagnosticsChanged();
private:
    Document* current()const;void recover();void saveSession();void runTest(nand::ScriptTool tool);
    std::shared_ptr<ExecutionResult> snapshot()const;
    void recordHardware(const QString& event);
    QVariantList hardwareTrace_;QString hardwareEvent_;
    QList<Document*> docs_;QVariantList files_;QString workspace_,output_;int active_=-1;bool busy_=false,vmMode_=false,hardwareMode_=false;
    nand::Cpu cpu_;nand::Vm vm_;nand::Hardware hardware_;QTimer recoveryTimer_;
    QTimer autosaveTimer_;QString lastError_;QVariantList searchResults_,diagnostics_;
    QFutureWatcher<WorkspaceResult> workspaceTask_;QFutureWatcher<QVariantList> searchTask_;
    QFutureWatcher<TaskResult> task_;QFutureWatcher<std::shared_ptr<ExecutionResult>> execution_;
    std::atomic_bool cancelled_{false};
    std::atomic_int keyboard_{0};
};
class ScreenProvider : public QQuickImageProvider {
public: explicit ScreenProvider(Studio* s):QQuickImageProvider(Image),studio(s){}
    QImage requestImage(const QString&,QSize* size,const QSize&)override{auto i=studio->screen();if(size)*size=i.size();return i;}
private:Studio* studio;
};
