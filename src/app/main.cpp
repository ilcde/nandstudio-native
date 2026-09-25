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
#include <QFileInfo>
#include <QDir>
#include <QFontDatabase>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <memory>
#ifdef Q_OS_ANDROID
#include <QJniObject>
#include <QStandardPaths>
#endif
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
#ifdef Q_OS_ANDROID
    // Device-test entry point: uses a fixed app-private report path, never an
    // arbitrary path supplied by another Android application.
    if(QNativeInterface::QAndroidApplication::isActivityContext()){
        QJniObject activity=QNativeInterface::QAndroidApplication::context();
        auto intent=activity.callObjectMethod("getIntent","()Landroid/content/Intent;");
        auto key=QJniObject::fromString("nandstudio.layoutCheck");
        if(intent.isValid()&&intent.callMethod<jboolean>("getBooleanExtra","(Ljava/lang/String;Z)Z",key.object<jstring>(),jboolean(false)))
            arguments << "--layout-report" << QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)+"/layout-report.json";
    }
#endif
    if(arguments.contains("--layout-report")){
        const int at=arguments.indexOf("--layout-report");if(at+1>=arguments.size())return 2;
        const auto destination=arguments[at+1];
        QTimer::singleShot(1500,&app,[&app,&engine,destination]{
            auto* window=qobject_cast<QQuickWindow*>(engine.rootObjects().first());
            if(!window){QCoreApplication::exit(1);return;}
            const auto margins=window->safeAreaMargins();
            const QRectF usable(margins.left(),margins.top(),window->width()-margins.left()-margins.right(),window->height()-margins.top()-margins.bottom());
            QJsonArray controls;bool passed=true;
            for(auto name:{"filesButton","saveButton","buildButton","moreButton"}){
                auto* item=window->findChild<QQuickItem*>(name);
                const auto rect=item?item->mapRectToScene(QRectF(0,0,item->width(),item->height())):QRectF();
                const bool inside=item&&item->isVisible()&&!rect.isEmpty()&&usable.contains(rect);passed&=inside;
                controls.append(QJsonObject{{"name",name},{"x",rect.x()},{"y",rect.y()},{"width",rect.width()},{"height",rect.height()},{"inside_safe_area",inside}});
            }
            QJsonObject report{{"platform",QGuiApplication::platformName()},{"width",window->width()},{"height",window->height()},{"safe_top",margins.top()},{"safe_bottom",margins.bottom()},{"safe_left",margins.left()},{"safe_right",margins.right()},{"controls",controls},{"passed",passed}};
            QDir().mkpath(QFileInfo(destination).absolutePath());QFile file(destination);
            if(!file.open(QIODevice::WriteOnly)||file.write(QJsonDocument(report).toJson())<0)passed=false;
            qInfo("Toolbar safe-area check: %s (top=%d)",passed?"passed":"FAILED",margins.top());
            QCoreApplication::exit(passed?0:1);
        });
        return app.exec();
    }
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
