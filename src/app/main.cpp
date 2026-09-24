// SPDX-License-Identifier: GPL-3.0-or-later
#include "studio.hpp"
#include "editor_services.hpp"
#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickStyle>
#include <QTimer>
#include <QQuickWindow>
#include <QFile>
#include <QDir>
#include <QFontDatabase>
#include <memory>
#ifdef NAND_GUI_CHECKS
void runGuiChecks(Studio&,QQmlApplicationEngine&,const QString&);
#endif
int main(int argc,char** argv){
    const auto testDir=qEnvironmentVariable("NAND_TEST_STATE_DIR");
    if(!testDir.isEmpty()){
        QDir().mkpath(testDir);
        qInstallMessageHandler([](QtMsgType,const QMessageLogContext&,const QString& text){QFile f(qEnvironmentVariable("NAND_TEST_STATE_DIR")+"/qt.log");if(f.open(QIODevice::Append)){f.write(text.toUtf8());f.write("\n");}});
    }
    qInfo("Creating application");
    QGuiApplication app(argc,argv);app.setOrganizationName("NandStudio");app.setApplicationName("NandStudio");
#ifdef Q_OS_WIN
    if(!testDir.isEmpty()){QFontDatabase::addApplicationFont("C:/Windows/Fonts/segoeui.ttf");QFontDatabase::addApplicationFont("C:/Windows/Fonts/consola.ttf");}
#endif
    qInfo("Creating studio");
    qmlRegisterType<LineNumberGutter>("NandStudio.Native",1,0,"LineNumberGutter");
    Preferences preferences;EditorServices editorTools;
    QQuickStyle::setStyle("Fusion");auto owner=std::make_unique<Studio>();auto& studio=*owner;QQmlApplicationEngine engine;
    qInfo("Loading QML");
    studio.setAutosaveSeconds(preferences.values()["autosaveSeconds"].toInt());
    QObject::connect(&preferences,&Preferences::changed,&studio,[&]{studio.setAutosaveSeconds(preferences.values()["autosaveSeconds"].toInt());});
    engine.rootContext()->setContextProperty("preferences",&preferences);engine.rootContext()->setContextProperty("editorTools",&editorTools);
    engine.rootContext()->setContextProperty("studio",&studio);engine.addImageProvider("screen",new ScreenProvider(&studio));
    engine.loadFromModule("NandStudio","Main");if(engine.rootObjects().isEmpty())return 1;
    qInfo("QML loaded");
    auto arguments=app.arguments();
#ifdef NAND_GUI_CHECKS
    if(arguments.contains("--self-test")){
        int pos=arguments.indexOf("--self-test");if(pos+1>=arguments.size())return 2;
        QTimer::singleShot(100,&app,[&studio,&engine,dir=arguments[pos+1]]{runGuiChecks(studio,engine,dir);});
        return app.exec();
    }
#endif
    for(const auto& arg:arguments.mid(1))if(!arg.startsWith("--"))studio.open(QUrl::fromLocalFile(arg));
    if(app.arguments().contains("--smoke-test"))QTimer::singleShot(2000,&app,&QCoreApplication::quit);
    return app.exec();
}
