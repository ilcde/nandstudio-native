// SPDX-License-Identifier: GPL-3.0-or-later
#include "app/studio.hpp"
#include "app/editor_services.hpp"
#include <QQmlContext>
#include <QQuickTextDocument>
#include <QQmlApplicationEngine>
#include <QQuickWindow>
#include <QQuickItem>
#include <QFile>
#include <QDir>
#include <QTest>
#include <QSignalSpy>
#include <QCoreApplication>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QFontDatabase>
#include <QKeyEvent>
#include <QDateTime>
#include <QDirIterator>
void runGuiChecks(Studio& studio,QQmlApplicationEngine& engine,const QString& dir){
    QJsonArray checks;int exitCode=0;QDir().mkpath(dir);
    auto check=[&](bool passed,const QString& label){checks.append(QJsonObject{{"check",label},{"passed",passed}});if(!passed)throw std::runtime_error(label.toStdString());};
    auto write=[](const QString& path,const QByteArray& bytes){QFile f(path);return f.open(QIODevice::WriteOnly)&&f.write(bytes)==bytes.size();};
    auto bytes=[](const QString& path){QFile f(path);if(!f.open(QIODevice::ReadOnly))return QByteArray{};return f.readAll();};
    try{
        auto path=QDir(dir).absoluteFilePath("Ui.asm");check(write(path,"@2\r\nD=A\r\n"),"create isolated fixture");studio.openWorkspace(QUrl::fromLocalFile(QDir(dir).absolutePath()));studio.open(QUrl::fromLocalFile(path));
        QTest::qWait(100);auto* window=qobject_cast<QQuickWindow*>(engine.rootObjects().first());check(window!=nullptr,"Qt Quick window instantiated");
        if(qEnvironmentVariableIsSet("NAND_LAYOUT_BASELINE")){
            std::function<QQuickItem*(QQuickItem*,const QString&,const QString&)> find=[&](QQuickItem* item,const QString& prop,const QString& value)->QQuickItem*{if(item->property(prop.toUtf8()).toString()==value)return item;for(auto* child:item->childItems())if(auto* found=find(child,prop,value))return found;return nullptr;};
            auto* button=find(window->contentItem(),"text","+ New file");if(!button)throw std::runtime_error("Create-file button missing");QTest::mouseClick(window,Qt::LeftButton,Qt::NoModifier,button->mapToScene(QPointF(button->width()/2,button->height()/2)).toPoint());QTest::qWait(80);
            auto* field=find(window->contentItem(),"placeholderText","Name.jack");if(!field)throw std::runtime_error("Filename field missing");auto* popup=field->parentItem();while(popup&&!QString(popup->metaObject()->className()).contains("PopupItem"))popup=popup->parentItem();if(!popup)throw std::runtime_error("Popup missing");
            auto bounds=popup->mapRectToScene(QRectF(0,0,popup->width(),popup->height()));auto child=field->mapRectToScene(QRectF(0,0,field->width(),field->height()));
            QJsonObject geometry{{"popup_width",bounds.width()},{"field_width",child.width()},{"field_inside_popup",bounds.contains(child)},{"overflow_reproduced",!bounds.contains(child)}};
            write(QDir(dir).filePath("dialog-before.json"),QJsonDocument(geometry).toJson());window->grabWindow().save(QDir(dir).filePath("dialog-before.png"));QCoreApplication::exit(0);return;
        }
        std::function<QQuickItem*(QQuickItem*)> findEditor=[&](QQuickItem* item)->QQuickItem*{if(item->objectName()=="editor_Ui.asm")return item;for(auto* child:item->childItems())if(auto* found=findEditor(child))return found;return nullptr;};
        auto* editor=findEditor(window->contentItem());check(editor!=nullptr,"editor reachable through document tab");editor->forceActiveFocus();QTest::qWait(30);
        auto letter=[&](int key,const QString& text){QKeyEvent press(QEvent::KeyPress,key,Qt::ShiftModifier,text);QCoreApplication::sendEvent(window,&press);QKeyEvent release(QEvent::KeyRelease,key,Qt::ShiftModifier,text);QCoreApplication::sendEvent(window,&release);};
        editor->setProperty("cursorPosition",editor->property("text").toString().size());QTest::keyClick(window,Qt::Key_At);QTest::keyClick(window,Qt::Key_0);QTest::keyClick(window,Qt::Key_Return);letter(Qt::Key_M,"M");QTest::keyClick(window,Qt::Key_Equal);letter(Qt::Key_D,"D");
        QTest::qWait(30);auto* doc=qvariant_cast<Document*>(studio.documents()[studio.active()]);check(doc&&doc->dirty(),"keyboard editing marks document dirty");
        check(doc->text().endsWith("@0\nM=D"),"keyboard editing reaches C++ document");
        QTest::keyClick(window,Qt::Key_Z,Qt::ControlModifier);QTest::qWait(20);check(!doc->text().endsWith("M=D"),"native editor undo");QTest::keyClick(window,Qt::Key_Y,Qt::ControlModifier);QTest::qWait(20);check(doc->text().endsWith("M=D"),"native editor redo");
        check(doc->save(),"save through document service");check(bytes(path)=="@2\r\nD=A\r\n@0\r\nM=D","preserve CRLF and final-newline state");
        doc->setText(doc->text()+"\n// buffer");check(write(path,"// external edit"),"simulate external writer");check(!doc->save(),"external change blocks overwrite");check(bytes(path)=="// external edit","external bytes unchanged");
        check(doc->resolveConflict("reload")&&doc->text()=="// external edit"&&!doc->dirty(),"conflict reload preserves reviewed disk version");
        auto* conflict=window->findChild<QObject*>("conflictDialog");check(conflict!=nullptr,"conflict workflow has a dialog");QMetaObject::invokeMethod(conflict,"reject");QTest::qWait(30);
        doc->setText("@2\nD=A\n@0\nM=D\n");check(doc->save(),"save reconciled document");studio.loadCpu();studio.step(4);for(int i=0;studio.busy()&&i<200;++i)QTest::qWait(10);check(!studio.busy()&&studio.memory(0)==2,"CPU execution updates memory inspector");
        studio.build();for(int i=0;studio.busy()&&i<200;++i)QTest::qWait(10);check(!studio.busy()&&QFile::exists(QDir(dir).filePath("Ui.hack")),"native assemble action emits artifact");
        auto hdlPath=QDir(dir).absoluteFilePath("Ui.hdl");check(write(hdlPath,"CHIP Ui { IN in[16], load; OUT out[16]; PARTS: Register(in=in,load=load,out=out); }"),"create independent HDL fixture");studio.open(QUrl::fromLocalFile(hdlPath));QTest::qWait(40);
        std::function<QQuickItem*(QQuickItem*,const QString&)> findItem=[&](QQuickItem* item,const QString& name)->QQuickItem*{if(item->objectName()==name)return item;for(auto* child:item->childItems())if(auto* found=findItem(child,name))return found;return nullptr;};
        auto wait=[&]{for(int i=0;studio.busy()&&i<300;++i)QTest::qWait(10);QTest::qWait(30);};
        auto click=[&](const QString& name){auto* item=findItem(window->contentItem(),name);if(!item)throw std::runtime_error("Missing control "+name.toStdString());QTest::mouseClick(window,Qt::LeftButton,Qt::NoModifier,item->mapToScene(QPointF(item->width()/2,item->height()/2)).toPoint());wait();};
        auto andPath=QDir(dir).filePath("And.hdl");write(andPath,"CHIP And { IN a,b; OUT out; BUILTIN And; }");studio.open(QUrl::fromLocalFile(andPath));QTest::qWait(30);click("loadHdl");
        for(auto name:{"a","b"}){auto* input=findItem(window->contentItem(),QString("pin_")+name);input->forceActiveFocus();QTest::keyClick(window,Qt::Key_A,Qt::ControlModifier);QTest::keyClick(window,Qt::Key_1);}
        click("hardwareEval");check(studio.hardwareValue("out")=="1","Eval commits typed input fields without Return and evaluates And");
        studio.open(QUrl::fromLocalFile(hdlPath));QTest::qWait(30);
        click("loadHdl");check(studio.state()["hardware"].toBool()&&studio.state()["chip"]=="Ui","Load HDL button loads native hierarchy");
        auto enterPin=[&](const QString& name,const QString& value){auto* item=findItem(window->contentItem(),"pin_"+name);if(!item)throw std::runtime_error("Missing pin editor");item->forceActiveFocus();item->setProperty("text",value);QTest::keyClick(window,Qt::Key_Return);QTest::qWait(30);};
        enterPin("in","123");enterPin("load","1");check(studio.hardwareValue("in")=="123","pin editor reaches hardware backend");click("hardwareTick");check(studio.state()["clockUp"].toBool()&&studio.hardwareValue("out")=="0","Tick samples without publishing register output");click("hardwareTock");check(!studio.state()["clockUp"].toBool()&&studio.hardwareValue("out")=="123","Tock publishes register output to inspector");
        auto* hdlDoc=qvariant_cast<Document*>(studio.documents()[studio.active()]);hdlDoc->setText("CHIP Ui { PARTS: Missing(");check(studio.hardwareValue("out")=="123","editing does not reset running hardware");click("loadHdl");check(studio.hardwareValue("out")=="123"&&studio.state()["chip"]=="Ui","failed explicit reload preserves last valid hardware");
        window->setProperty("dark",false);QTest::qWait(30);check(!window->property("dark").toBool(),"theme changes");window->setProperty("dark",true);
        auto image=window->grabWindow();check(!image.isNull()&&image.save(QDir(dir).filePath("desktop.png")),"desktop frame rendered");
        auto* preferences=qobject_cast<Preferences*>(engine.rootContext()->contextProperty("preferences").value<QObject*>());
        check(preferences!=nullptr,"persistent preferences service available");
        for(auto size:QList<QSize>{{320,640},{412,820},{820,412},{768,1024},{1320,860}}){
            window->resize(size);window->setProperty("mobilePane",0);QTest::qWait(40);
            for(double scale:{1.0,1.5}){
                preferences->set("uiScale",scale);QTest::qWait(40);QMetaObject::invokeMethod(window,"showNewFile");QTest::qWait(40);
                auto* field=findItem(window->contentItem(),"pathField");check(field!=nullptr,"filename field exists");
                auto* popup=field->parentItem();while(popup&&!QString(popup->metaObject()->className()).contains("PopupItem"))popup=popup->parentItem();check(popup!=nullptr,"file dialog surface exists");
                auto bounds=popup->mapRectToScene(QRectF(0,0,popup->width(),popup->height()));auto child=field->mapRectToScene(QRectF(0,0,field->width(),field->height()));
                auto label=QString("%1x%2 scale %3").arg(size.width()).arg(size.height()).arg(scale);
                check(bounds.contains(child),"filename field within dialog "+label);
                check(QRectF(0,0,window->width(),window->height()).contains(bounds),"dialog within window "+label);
                for(auto name:{"pathCancel","pathSubmit"}){auto* button=findItem(window->contentItem(),name);check(button&&bounds.contains(button->mapRectToScene(QRectF(0,0,button->width(),button->height()))),QString(name)+" within dialog "+label);}
                field->setProperty("text",QString(160,'X')+".jack");QTest::qWait(20);check(bounds.contains(field->mapRectToScene(QRectF(0,0,field->width(),field->height()))),"long filename does not expand dialog "+label);
                check(window->grabWindow().save(QDir(dir).filePath(QString("dialog-%1-%2-%3.png").arg(size.width()).arg(size.height()).arg(scale))),"dialog screenshot "+label);
                QTest::keyClick(window,Qt::Key_Escape);QTest::qWait(20);
            }
        }
        for(auto size:QList<QSize>{{320,640},{820,412}}){window->resize(size);preferences->set("uiScale",1.5);QTest::qWait(30);
            for(auto name:{"lineDialog","settingsDialog","closeDocumentDialog","exitDialog","trashDialog","conflictDialog"}){
                auto* dialog=window->findChild<QObject*>(name);check(dialog!=nullptr,QString(name)+" reachable");QMetaObject::invokeMethod(dialog,"open");QTest::qWait(30);
                auto* content=dialog->property("contentItem").value<QQuickItem*>();auto* popup=content;while(popup&&!QString(popup->metaObject()->className()).contains("PopupItem"))popup=popup->parentItem();
                check(popup&&QRectF(0,0,window->width(),window->height()).contains(popup->mapRectToScene(QRectF(0,0,popup->width(),popup->height()))),QString(name)+" bounded at "+QString::number(size.width()));
                QMetaObject::invokeMethod(dialog,"reject");QTest::qWait(20);
            }
        }
        preferences->set("uiScale",1.0);window->resize(1320,860);QTest::qWait(40);
        studio.open(QUrl::fromLocalFile(path));QTest::qWait(40);editor=findEditor(window->contentItem());
        auto* services=qobject_cast<EditorServices*>(engine.rootContext()->contextProperty("editorTools").value<QObject*>());
        auto* textDocument=editor->property("textDocument").value<QQuickTextDocument*>();
        doc->setText("let a = 1;\nlet aa = 2;\n// a\n");QTest::qWait(30);
        check(services->replaceAll(textDocument,"a","counter",true,true)==2&&doc->text().contains("let aa"),"whole-word replace all uses document model");
        editor->forceActiveFocus();QTest::keyClick(window,Qt::Key_Z,Qt::ControlModifier);QTest::qWait(20);check(doc->text()=="let a = 1;\nlet aa = 2;\n// a\n","replace all is one undo operation");
        services->indent(textDocument,0,doc->text().size(),2,false,false);check(doc->text().startsWith("  let")&&doc->text().contains("\n  let"),"selected lines indent together");
        services->indent(textDocument,0,doc->text().size(),2,false,true);check(doc->text().startsWith("let"),"selected lines unindent together");
        services->comment(textDocument,0,doc->text().indexOf('\n'));check(doc->text().startsWith("// let"),"comment toggling uses C++ editor service");services->comment(textDocument,0,doc->text().indexOf('\n'));check(doc->text().startsWith("let"),"comment toggling reverses");
        check(services->matchingBracket("{ /* } */ \"}\" }",1)==14,"bracket matching skips comments and strings");
        auto invalidPath=QDir(dir).filePath("Invalid.asm");check(write(invalidPath,"D=INVALID"),"create parser-negative GUI fixture");studio.open(QUrl::fromLocalFile(invalidPath));studio.build();studio.open(QUrl::fromLocalFile(path));wait();
        check(!studio.diagnostics().isEmpty()&&studio.diagnostics().last().toMap()["path"].toString()==invalidPath,"worker diagnostics retain originating document");
        check(qvariant_cast<Document*>(studio.documents()[studio.active()])->path()==invalidPath,"diagnostic navigation selects error document after active tab changes");
        check(!studio.createFile("../escape.asm"),"workspace creation rejects traversal");
        auto newName="Created-"+QString::number(QDateTime::currentMSecsSinceEpoch())+".asm";
        QMetaObject::invokeMethod(window,"showNewFile");QTest::qWait(40);auto* newField=findItem(window->contentItem(),"pathField");newField->setProperty("text",newName);newField->forceActiveFocus();QTest::keyClick(window,Qt::Key_Return);QTest::qWait(50);
        check(QFile::exists(QDir(dir).filePath(newName)),"Return in Create File creates a real workspace file");
        auto* newDoc=qvariant_cast<Document*>(studio.documents()[studio.active()]);newDoc->setText("// searchableUnsavedMarker");
        check(!studio.trashFile(newName),"trash refuses unsaved documents");
        check(studio.renameFile(newName,"Renamed-"+newName)&&newDoc->name()=="Renamed-"+newName,"rename follows the open document");
        studio.refreshWorkspace();QTest::qWait(100);studio.findInProject("searchableUnsavedMarker");for(int i=0;studio.searchResults().isEmpty()&&i<100;++i)QTest::qWait(10);
        check(!studio.searchResults().isEmpty()&&studio.searchResults().first().toMap()["line"]==1,"project search reads unsaved buffers with locations");
        check(newDoc->save()&&studio.trashFile("Renamed-"+newName),"saved file moves to recoverable workspace trash");
        check(!QFile::exists(QDir(dir).filePath("Renamed-"+newName)),"trashed file leaves its original location");
        bool recoveredBytes=false;QDirIterator trash(QDir(dir).filePath(".nandstudio-trash"),QDir::Files,QDirIterator::Subdirectories);while(trash.hasNext()){auto p=trash.next();if(p.endsWith(newName))recoveredBytes=bytes(p)=="// searchableUnsavedMarker";}
        check(recoveredBytes,"recovery trash preserves file bytes");
        studio.open(QUrl::fromLocalFile(path));doc->setText("// edited buffer");check(write(path,"// disk version one"),"prepare second save conflict");check(!doc->save(),"second conflict is detected");check(write(path,"// disk version two"),"external writer changes reviewed version");
        check(!doc->resolveConflict("overwrite")&&bytes(path)=="// disk version two","overwrite stops if disk changed again");check(doc->resolveConflict("overwrite")&&bytes(path)=="// edited buffer","explicit overwrite saves only reviewed conflict version");QMetaObject::invokeMethod(conflict,"reject");
        window->resize(412,820);QTest::qWait(40);check(window->property("compact").toBool(),"phone-width responsive navigation enabled");window->setProperty("mobilePane",2);QTest::qWait(40);check(window->grabWindow().save(QDir(dir).filePath("phone-width.png")),"phone-width machine pane rendered");
        check(preferences->values()["recentWorkspaces"].toStringList().contains(studio.workspace()),"recent workspaces contain opened local folder");
        Preferences reloadedPreferences;check(reloadedPreferences.values()["recentWorkspaces"]==preferences->values()["recentWorkspaces"],"recent workspaces survive settings reload");
        auto runtimeLog=bytes(qEnvironmentVariable("NAND_TEST_STATE_DIR")+"/qt.log");runtimeLog=runtimeLog.mid(runtimeLog.lastIndexOf("Creating application"));
        check(!runtimeLog.contains("Binding loop")&&!runtimeLog.contains("TypeError")&&!runtimeLog.contains("ReferenceError")&&!runtimeLog.contains("QDataStream::operator"),"no runtime QML binding, type, or settings serialization errors");
    }catch(const std::exception& e){qWarning("GUI check failed: %s",e.what());exitCode=1;}
    write(QDir(dir).filePath("checks.json"),QJsonDocument(checks).toJson());QCoreApplication::exit(exitCode);
}
