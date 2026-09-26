// SPDX-License-Identifier: GPL-3.0-or-later
#include "app/studio.hpp"
#include "app/editor_services.hpp"
#include "app/storage.hpp"
#include "builtin_hdl.hpp"
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
#include <QTemporaryDir>
void runGuiChecks(Studio& studio,QQmlApplicationEngine& engine,const QString& dir){
    QJsonArray checks;int exitCode=0;QDir().mkpath(dir);
    auto check=[&](bool passed,const QString& label){checks.append(QJsonObject{{"check",label},{"passed",passed}});QFile progress(QDir(dir).filePath("checks.json"));if(progress.open(QIODevice::WriteOnly))progress.write(QJsonDocument(checks).toJson());if(!passed)throw std::runtime_error(label.toStdString());};
    auto write=[](const QString& path,const QByteArray& bytes){QFile f(path);return f.open(QIODevice::WriteOnly)&&f.write(bytes)==bytes.size();};
    auto bytes=[](const QString& path){QFile f(path);if(!f.open(QIODevice::ReadOnly))return QByteArray{};return f.readAll();};
    try{
        auto path=QDir(dir).absoluteFilePath("Ui.asm");check(write(path,"@2\r\nD=A\r\n"),"create isolated fixture");studio.openWorkspace(QUrl::fromLocalFile(QDir(dir).absolutePath()));studio.open(QUrl::fromLocalFile(path));
        QTest::qWait(100);auto* window=qobject_cast<QQuickWindow*>(engine.rootObjects().first());check(window!=nullptr,"Qt Quick window instantiated");
        check(QTest::qWaitForWindowExposed(window,5000),"GUI test window is exposed for native input");
        for(auto name:{"openWorkspaceMenuItem","openFileMenuItem","settingsMenuItem"}){
            auto* item=window->findChild<QObject*>(name);
            const auto prefix=QString(name)=="openWorkspaceMenuItem" ? "Open workspace" : QString(name)=="openFileMenuItem" ? "Open file" : "Settings";
            check(item&&item->property("text").toString()==QString(prefix)+QChar(0x2026),QString("menu punctuation renders correctly: ")+name);
        }
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
        auto* searchBar=window->findChild<QQuickItem*>("findReplaceBar");
        check(searchBar&&!searchBar->isVisible(),"find and replace is hidden until requested");
        check(searchBar->property("editor").value<QObject*>()!=nullptr,"find bar tracks the first asynchronously created editor");
        auto letter=[&](int key,const QString& text){QKeyEvent press(QEvent::KeyPress,key,Qt::ShiftModifier,text);QCoreApplication::sendEvent(window,&press);QKeyEvent release(QEvent::KeyRelease,key,Qt::ShiftModifier,text);QCoreApplication::sendEvent(window,&release);};
        editor->setProperty("cursorPosition",editor->property("text").toString().size());QTest::keyClick(window,Qt::Key_At);QTest::keyClick(window,Qt::Key_0);QTest::keyClick(window,Qt::Key_Return);letter(Qt::Key_M,"M");QTest::keyClick(window,Qt::Key_Equal);letter(Qt::Key_D,"D");
        QTest::qWait(30);auto* doc=qvariant_cast<Document*>(studio.documents()[studio.active()]);check(doc&&doc->dirty(),"keyboard editing marks document dirty");
        check(doc->text().endsWith("@0\nM=D"),"keyboard editing reaches C++ document");
        QTest::keySequence(window,QKeySequence(QKeySequence::Undo));QTest::qWait(20);check(!doc->text().endsWith("M=D"),"native editor undo");QTest::keySequence(window,QKeySequence(QKeySequence::Redo));QTest::qWait(20);check(doc->text().endsWith("M=D"),"native editor redo");
        check(doc->save(),"save through document service");check(bytes(path)=="@2\r\nD=A\r\n@0\r\nM=D","preserve CRLF and final-newline state");
        doc->setText(doc->text()+"\n// buffer");check(write(path,"// external edit"),"simulate external writer");check(!doc->save(),"external change blocks overwrite");check(bytes(path)=="// external edit","external bytes unchanged");
        check(doc->resolveConflict("reload")&&doc->text()=="// external edit"&&!doc->dirty(),"conflict reload preserves reviewed disk version");
        auto* conflict=window->findChild<QObject*>("conflictDialog");check(conflict!=nullptr,"conflict workflow has a dialog");QMetaObject::invokeMethod(conflict,"reject");QTest::qWait(30);
        doc->setText("@2\nD=A\n@0\nM=D\n");check(doc->save(),"save reconciled document");studio.loadCpu();studio.step(4);for(int i=0;studio.busy()&&i<200;++i)QTest::qWait(10);check(!studio.busy()&&studio.memory(0)==2,"CPU execution updates memory inspector");
        studio.build();for(int i=0;studio.busy()&&i<200;++i)QTest::qWait(10);check(!studio.busy()&&QFile::exists(QDir(dir).filePath("Ui.hack")),"native assemble action emits artifact");
        auto hdlPath=QDir(dir).absoluteFilePath("Ui.hdl");check(write(hdlPath,"CHIP Ui { IN in[16], load; OUT out[16]; PARTS: Register(in=in,load=load,out=out); }"),"create independent HDL fixture");studio.open(QUrl::fromLocalFile(hdlPath));QTest::qWait(40);
        std::function<QQuickItem*(QQuickItem*,const QString&)> findItem=[&](QQuickItem* item,const QString& name)->QQuickItem*{if(item->objectName()==name)return item;for(auto* child:item->childItems())if(auto* found=findItem(child,name))return found;return nullptr;};
        auto wait=[&]{for(int i=0;studio.busy()&&i<300;++i)QTest::qWait(10);QTest::qWait(30);};
        studio.open(QUrl::fromLocalFile(path));studio.loadCpu();studio.setBreakpoint(2,true);studio.step(100);wait();
        check(studio.state()["PC"]==2&&studio.state()["pauseReason"].toString().contains("Breakpoint"),"CPU run stops before executing breakpoint instruction");
        check(studio.cpuInstruction()["decoded"]=="@0"&&studio.cpuInstruction()["line"]==3&&studio.cpuInstruction()["canNavigate"].toBool(),"CPU inspector decodes instruction and maps PC to original ASM line");
        studio.step(1);wait();check(studio.state()["PC"]==3,"single step advances past a breakpoint");studio.setBreakpoint(2,false);
        studio.addWatch("D");studio.addWatch("RAM[0]");studio.addWatch("missing");auto watches=studio.watchValues();
        check(watches.size()==3&&watches[0].toMap()["value"].toString().startsWith("2 / 0x0002")&&!watches[2].toMap()["value"].toString().isEmpty(),"live watches show machine state and safe expression errors");
        studio.removeWatch("D");studio.removeWatch("RAM[0]");studio.removeWatch("missing");
        check(studio.convertWord("-1",10)["binary"]=="1111111111111111"&&studio.convertWord("ffff",16)["signed"]==-1,"word converter preserves signed two's complement");
        check(studio.convertWord("1000000000000000",2)["signed"]==-32768&&studio.convertWord("65536",10).contains("error")&&studio.convertWord("2",2).contains("error"),"word converter validates radix and 16-bit bounds");
        studio.open(QUrl::fromLocalFile(path));wait();
        auto previewWait=[&]{for(int i=0;studio.conversion()["busy"].toBool()&&i<200;++i)QTest::qWait(10);check(!studio.conversion()["busy"].toBool(),"conversion worker completes");};
        auto* previewDoc=qvariant_cast<Document*>(studio.documents()[studio.active()]);
        auto diskBeforePreview=bytes(path);previewDoc->setText("@3\nD=A\n");
        check(!studio.cpuInstruction()["canNavigate"].toBool()&&studio.cpuInstruction()["sourceChanged"].toBool(),"CPU source navigation is disabled when editor differs from loaded snapshot");
        studio.previewConversion();previewWait();
        check(studio.conversion()["output"]=="0000000000000011\n1110110000010000\n"&&bytes(path)==diskBeforePreview,"ASM preview consumes unsaved buffer without writing source");
        previewDoc->setText("D=INVALID\n");check(studio.conversion()["stale"].toBool()&&studio.conversion()["output"].toString().isEmpty(),"editing invalidates previous conversion preview");
        studio.previewConversion();previewWait();check(studio.conversion()["error"].toBool()&&studio.conversion()["line"].toInt()==1,"preview parser returns a navigable diagnostic");
        previewDoc->setText("@3\nD=A\n");studio.previewConversion();previewWait();
        bool previewError=false;for(const auto& entry:studio.diagnostics())if(entry.toMap()["source"]=="preview")previewError=true;
        check(!previewError&&!studio.conversion()["error"].toBool(),"successful preview replaces its previous diagnostic");
        auto* toolsPanel=window->findChild<QObject*>("toolsDialog");check(toolsPanel&&window->findChild<QObject*>("toolsMenuItem"),"converter panel is reachable from More");
        QMetaObject::invokeMethod(toolsPanel,"open");QTest::qWait(30);
        check(findItem(window->contentItem(),"converterResult")!=nullptr,"number converter renders in native tools panel");
        auto* liveCheck=findItem(window->contentItem(),"liveDiagnostics");check(liveCheck!=nullptr,"live diagnostics switch is reachable");liveCheck->setProperty("checked",true);
        QMetaObject::invokeMethod(toolsPanel,"close");previewDoc->setText("D=INVALID\n");QTest::qWait(750);previewWait();
        check(studio.conversion()["error"].toBool(),"live diagnostics runs after editing with the tools panel closed");
        liveCheck->setProperty("checked",false);
        previewDoc->setText("@3\n");studio.previewConversion();previewDoc->setText("@4\n");previewWait();
        check(studio.conversion()["stale"].toBool()&&studio.conversion()["output"].toString().isEmpty(),"worker result from an older editor snapshot is discarded");
        previewDoc->setText(QString::fromUtf8(diskBeforePreview).replace("\r\n","\n"));
        const auto previewJack=QDir(dir).absoluteFilePath("Preview.jack");write(previewJack,"class Preview { function void main() { return; } }");studio.open(QUrl::fromLocalFile(previewJack));
        studio.previewConversion();previewWait();check(studio.conversion()["output"]=="function Preview.main 0\npush constant 0\nreturn\n"&&!QFile::exists(QDir(dir).filePath("Preview.vm")),"Jack preview produces exact VM without generating a file");
        QTemporaryDir vmPreviewFolder(QDir(dir).filePath("vm-preview-XXXXXX"));check(vmPreviewFolder.isValid(),"create isolated VM conversion workspace");
        const auto vmPreviewPath=QDir(vmPreviewFolder.path()).filePath("Main.vm");write(vmPreviewPath,"push constant 7\npush constant 8\nadd\n");studio.open(QUrl::fromLocalFile(vmPreviewPath));
        studio.previewConversion();previewWait();check(!studio.conversion()["error"].toBool()&&!nand::assemble(studio.conversion()["output"].toString().toStdString()).words.empty(),"VM folder preview produces valid native assembly");
        studio.build();wait();check(QFile::exists(QDir(vmPreviewFolder.path()).filePath("Main.asm")),"Build VM exposes its assembly artifact in the editor");
        write(vmPreviewPath,"function Sys.init 0\ncall Other.value 0\npop temp 0\nlabel END\ngoto END\n");
        const auto otherVm=QDir(vmPreviewFolder.path()).filePath("Other.vm");write(otherVm,"function Other.value 0\npush constant 9\nreturn\n");
        studio.open(QUrl::fromLocalFile(otherVm));qvariant_cast<Document*>(studio.documents()[studio.active()])->setText("function Other.value 0\npush constant 11\nreturn\n");
        studio.open(QUrl::fromLocalFile(vmPreviewPath));qvariant_cast<Document*>(studio.documents()[studio.active()])->setText(QString::fromUtf8(bytes(vmPreviewPath)));
        studio.build(true,true);wait();const auto folderAsm=QDir(vmPreviewFolder.path()).filePath(QFileInfo(vmPreviewFolder.path()).fileName()+".asm");
        auto translated=nand::assemble(bytes(folderAsm).toStdString());nand::Cpu translatedCpu;translatedCpu.load(translated.words);int translatedSteps=0;while(translatedCpu.pc!=translated.symbols.at("Sys.init$END")&&translatedSteps++<10000)translatedCpu.step();
        check(translatedCpu.ram[5]==11&&bytes(otherVm).contains("constant 9"),"VM folder translation uses unsaved dependency snapshot with bootstrap without saving originals");
        auto click=[&](const QString& name){auto* item=findItem(window->contentItem(),name);if(!item)throw std::runtime_error("Missing control "+name.toStdString());QTest::mouseClick(window,Qt::LeftButton,Qt::NoModifier,item->mapToScene(QPointF(item->width()/2,item->height()/2)).toPoint());wait();};
        auto xorFolder=QDir(dir).filePath("requested-xor");QDir().mkpath(xorFolder);QFile::remove(QDir(xorFolder).filePath("Not.hdl"));auto xorPath=QDir(xorFolder).filePath("Xor.hdl");
        const QByteArray xorSource="CHIP Xor { IN a,b; OUT out; PARTS: Not(in=a,out=Nota); Not(in=b,out=Notb); And(a=a,b=Notb,out=aAndNotb); And(a=Nota,b=b,out=NotaAndb); Or(a=aAndNotb,b=NotaAndb,out=out); }";
        write(xorPath,xorSource);studio.open(QUrl::fromLocalFile(xorPath));wait();
        studio.evaluateHardwareWithInputs({{"a",1},{"b",0}});wait();check(studio.hardwareValue("out")=="1","first Eval loads and evaluates the user's composite Xor");
        studio.previewConversion();previewWait();check(!studio.conversion()["error"].toBool()&&studio.hardwareValue("out")=="1","HDL hierarchy validation preserves live circuit outputs");
        for(int a=0;a<2;++a)for(int b=0;b<2;++b){
            for(auto pair:{qMakePair(QString("a"),a),qMakePair(QString("b"),b)}){auto* field=findItem(window->contentItem(),"pin_"+pair.first);if(!field)throw std::runtime_error("Xor pin missing");field->forceActiveFocus();QTest::keySequence(window,QKeySequence::SelectAll);QTest::keyClick(window,pair.second?Qt::Key_1:Qt::Key_0);}
            click("hardwareEval");check(studio.hardwareValue("out")==QString::number(a^b),QString("composite Xor Eval truth table %1,%2").arg(a).arg(b));
        }
        auto* xorDocument=qvariant_cast<Document*>(studio.documents()[studio.active()]);xorDocument->setText(QString::fromUtf8(xorSource).replace("Or(a=aAndNotb","And(a=aAndNotb"));QTest::qWait(30);
        studio.open(QUrl::fromLocalFile(path));wait();check(studio.hardwareNeedsReload(),"changed HDL remains reloadable when a non-HDL tab is active");
        studio.evaluateHardwareWithInputs({{"a",1},{"b",0}});wait();check(studio.hardwareValue("out")=="0","Eval reloads the running HDL snapshot from a non-HDL tab");
        studio.open(QUrl::fromLocalFile(xorPath));wait();xorDocument->setText(QString::fromUtf8(xorSource));studio.evaluateHardwareWithInputs({{"a",0},{"b",0}});wait();
        click("togglePin_b");click("hardwareEval");check(studio.hardwareValue("out")=="1"&&studio.hardwareEvaluation()=="Eval completed: out=1","tap input and Eval publish an explicit live Xor result");
        xorDocument->setText(QString::fromUtf8(xorSource).replace("Or(a=aAndNotb","And(a=aAndNotb"));QTest::qWait(30);
        check(studio.hardwareNeedsReload()&&findItem(window->contentItem(),"hardwareEval")->property("text")=="Reload & Eval","changed HDL explicitly offers Reload and Eval");
        studio.evaluateHardwareWithInputs({{"a",1},{"b",0}});wait();check(studio.hardwareValue("out")=="0"&&bytes(xorPath)==xorSource,"Reload and Eval uses unsaved source without saving it");
        xorDocument->setText(QString::fromUtf8(xorSource));studio.evaluateHardwareWithInputs({{"a",1},{"b",0}});wait();
        xorDocument->setText("CHIP Xor { PARTS: Not(");studio.evaluateHardwareWithInputs({});wait();
        check(studio.hardwareValue("out")=="1"&&!studio.diagnostics().isEmpty()&&studio.diagnostics().last().toMap()["path"]==xorPath,"incomplete HDL preserves running chip and reports parser location");
        check(studio.hardwareEvaluation().startsWith("Hardware action failed:")&&!studio.hardwareEvaluation().contains("Eval completed"),"failed reload replaces stale success feedback");
        xorDocument->setText(QString::fromUtf8(xorSource)+"\n// reload unchanged circuit\n");
        studio.hardwareAction("eval");wait();
        check(studio.hardwareValue("a")=="1"&&studio.hardwareValue("b")=="0"&&studio.hardwareValue("out")=="1","Eval without pin arguments retains inputs when reloading the same circuit");
        xorDocument->setText(QString::fromUtf8(xorSource));write(QDir(xorFolder).filePath("Not.hdl"),"CHIP Not { IN in; OUT out; PARTS: }");studio.loadHardware();wait();studio.hardwareActionWithInputs("eval",{{"a",1},{"b",0}});wait();
        check(studio.hardwareValue("out")=="0"&&studio.hardwareMessage().contains("Not has an empty PARTS"),"unfinished local dependencies keep legacy precedence and show a warning");
        const auto localNot=QDir(xorFolder).filePath("Not.hdl");
        write(localNot,"CHIP Not { IN in; OUT out; PARTS: Nand(a=in,b=in,out=out); }");
        studio.evaluateHardwareWithInputs({{"a",1},{"b",0}});wait();
        check(studio.hardwareValue("out")=="1","Eval reloads externally changed dependency and computes Xor");
        write(localNot,"CHIP Not { IN in; OUT out; PARTS: }");studio.evaluateHardwareWithInputs({{"a",1},{"b",0}});wait();
        check(studio.hardwareValue("out")=="0","Eval notices a later closed dependency change");
        QFile::remove(localNot);studio.evaluateHardwareWithInputs({{"a",1},{"b",0}});wait();
        check(studio.hardwareValue("out")=="1","Eval detects dependency removal and uses normal builtin lookup");
        write(localNot,"CHIP Not { IN in; OUT out; PARTS: }");studio.evaluateHardwareWithInputs({{"a",1},{"b",0}});wait();
        check(studio.hardwareValue("out")=="0","Eval detects a newly added local dependency and preserves its precedence");
        studio.loadHardware();wait();check(studio.hardwareEvaluation().isEmpty(),"explicit Load clears previous Eval result");
        auto andPath=QDir(dir).filePath("And.hdl");write(andPath,"CHIP And { IN a,b; OUT out; BUILTIN And; }");studio.open(QUrl::fromLocalFile(andPath));QTest::qWait(30);click("loadHdl");
        for(auto name:{"a","b"}){auto* input=findItem(window->contentItem(),QString("pin_")+name);if(!input)throw std::runtime_error("And input control missing after Load HDL");input->forceActiveFocus();QTest::keyClick(window,Qt::Key_A,Qt::ControlModifier);QTest::keyClick(window,Qt::Key_1);}
        click("hardwareEval");check(studio.hardwareValue("out")=="1","Eval commits typed input fields without Return and evaluates And");
        struct EvalCase { const char* chip; QVariantMap inputs; QVariantMap outputs; };
        const std::vector<EvalCase> evalCases={
            {"Nand",{{"a",1},{"b",1}},{{"out",0}}},
            {"Not",{{"in",-1}},{{"out",2}}},
            {"And",{{"a",1},{"b",1}},{{"out",1}}},
            {"Or",{{"a",0},{"b",1}},{{"out",1}}},
            {"Xor",{{"a",1},{"b",1}},{{"out",0}}},
            {"Mux",{{"a",0},{"b",1},{"sel",1}},{{"out",1}}},
            {"DMux",{{"in",1},{"sel",1}},{{"a",0},{"b",1}}},
            {"Not16",{{"in",123}},{{"out",-124}}},
            {"And16",{{"a",15},{"b",6}},{{"out",6}}},
            {"Or16",{{"a",8},{"b",3}},{{"out",11}}},
            {"Mux16",{{"a",8},{"b",3},{"sel",1}},{{"out",3}}},
            {"Or8Way",{{"in",128}},{{"out",1}}},
            {"Mux4Way16",{{"a",1},{"b",2},{"c",3},{"d",4},{"sel",2}},{{"out",3}}},
            {"Mux8Way16",{{"h",123},{"sel",7}},{{"out",123}}},
            {"DMux4Way",{{"in",1},{"sel",2}},{{"a",0},{"b",0},{"c",1},{"d",0}}},
            {"DMux8Way",{{"in",1},{"sel",7}},{{"a",0},{"h",1}}},
            {"HalfAdder",{{"a",1},{"b",1}},{{"sum",0},{"carry",1}}},
            {"FullAdder",{{"a",1},{"b",1},{"c",1}},{{"sum",1},{"carry",1}}},
            {"Add16",{{"a",32767},{"b",1}},{{"out",-32768}}},
            {"Inc16",{{"in",-1}},{{"out",0}}},
            {"ALU",{{"x",12},{"y",7},{"f",1}},{{"out",19},{"zr",0},{"ng",0}}}
        };
        for(const auto& test:evalCases){
            QString file=QDir(dir).filePath(QString(test.chip)+".hdl");
            for(auto [name,source]:nand::builtinHdl)if(name==test.chip)write(file,QByteArray(source.data(),qsizetype(source.size())));
            studio.open(QUrl::fromLocalFile(file));QTest::qWait(15);click("loadHdl");
            for(auto it=test.inputs.cbegin();it!=test.inputs.cend();++it){
                auto* field=findItem(window->contentItem(),"pin_"+it.key());
                if(!field)throw std::runtime_error("Missing input for "+std::string(test.chip));
                field->forceActiveFocus();QTest::keyClick(window,Qt::Key_A,Qt::ControlModifier);
                for(auto character:it.value().toString())QTest::keyClick(window,Qt::Key(character.unicode()));
            }
            click("hardwareEval");
            for(auto it=test.outputs.cbegin();it!=test.outputs.cend();++it)
                check(studio.hardwareValue(it.key())==it.value().toString(),QString("Eval button commits pending inputs: %1.%2").arg(test.chip,it.key()));
            check(studio.state()["hardwareTime"].toString()=="0 ",QString("Eval leaves clock unchanged: %1").arg(test.chip));
        }
        check(studio.hardwareTrace().size()==2,"hardware history resets on chip load and records explicit Eval");
        auto latest=studio.hardwareTrace().last().toMap();
        check(latest["event"]=="eval"&&latest["values"].toMap()["out"]==19,"history contains actual backend pin snapshot");
        check(findItem(window->contentItem(),"hardwareWaveform")!=nullptr,"live hardware waveform view instantiated");
        check(studio.formatWord(-1,16)=="FFFF"&&studio.formatWord(-1,2)=="1111111111111111"&&studio.formatWord(65535,10)=="-1","numeric views preserve signed 16-bit values");
        auto* invalidInput=findItem(window->contentItem(),"pin_x");invalidInput->setProperty("text","invalid");
        click("hardwareEval");check(studio.hardwareValue("out")=="19"&&studio.hardwareValue("x")=="12","invalid pending pin input preserves prior valid state");
        studio.open(QUrl::fromLocalFile(hdlPath));QTest::qWait(30);
        click("loadHdl");check(studio.state()["hardware"].toBool()&&studio.state()["chip"]=="Ui","Load HDL button loads native hierarchy");
        auto* clockLabel=findItem(window->contentItem(),"hardwareClockLabel");
        check(clockLabel&&clockLabel->property("text").toString()==QString("Ui ")+QChar(0x00b7)+" time "+studio.state()["hardwareTime"].toString(),"hardware clock separator renders correctly");
        auto diagram=studio.hardwareDiagram("Ui");
        check(diagram["blocks"].toList().size()==2&&diagram["wires"].toList().size()==3,"diagram uses actual composite and Register connections");
        check(findItem(window->contentItem(),"hardwareDiagram")!=nullptr,"hierarchical connection visualization reachable");
        auto enterPin=[&](const QString& name,const QString& value){auto* item=findItem(window->contentItem(),"pin_"+name);if(!item)throw std::runtime_error("Missing pin editor");item->forceActiveFocus();item->setProperty("text",value);QTest::keyClick(window,Qt::Key_Return);QTest::qWait(30);};
        enterPin("in","123");enterPin("load","1");check(studio.hardwareValue("in")=="123","pin editor reaches hardware backend");click("hardwareTick");check(studio.state()["clockUp"].toBool()&&studio.hardwareValue("out")=="0","Tick samples without publishing register output");click("hardwareTock");check(!studio.state()["clockUp"].toBool()&&studio.hardwareValue("out")=="123","Tock publishes register output to inspector");
        auto history=studio.hardwareTrace();
        check(history.size()==3&&history[1].toMap()["clock"].toBool()&&!history[2].toMap()["clock"].toBool(),"history distinguishes Tick and Tock phases");
        check(history[1].toMap()["values"].toMap()["out"]==0&&history[2].toMap()["values"].toMap()["out"]==123,"waveform snapshots preserve sequential timing");
        studio.clearHardwareTrace();check(studio.hardwareTrace().size()==1&&studio.hardwareValue("out")=="123","clearing history preserves simulation state");
        auto* hdlDoc=qvariant_cast<Document*>(studio.documents()[studio.active()]);hdlDoc->setText("CHIP Ui { PARTS: Missing(");check(studio.hardwareValue("out")=="123","editing does not reset running hardware");click("loadHdl");check(studio.hardwareValue("out")=="123"&&studio.state()["chip"]=="Ui","failed explicit reload preserves last valid hardware");
        window->setProperty("dark",false);QTest::qWait(30);check(!window->property("dark").toBool(),"theme changes");window->setProperty("dark",true);
        auto image=window->grabWindow();check(!image.isNull()&&image.save(QDir(dir).filePath("desktop.png")),"desktop frame rendered");
        auto* preferences=qobject_cast<Preferences*>(engine.rootContext()->contextProperty("preferences").value<QObject*>());
        check(preferences!=nullptr,"persistent preferences service available");
        for(auto size:QList<QSize>{{320,640},{412,820},{820,412},{768,1024},{1320,860}}){
            window->resize(size);window->setProperty("mobilePane",0);QTest::qWait(40);
            for(double scale:{1.0,1.5}){
                preferences->set("uiScale",scale);QTest::qWait(150);QMetaObject::invokeMethod(window,"showNewFile");QTest::qWait(150);
                auto* field=findItem(window->contentItem(),"pathField");check(field!=nullptr,"filename field exists");
                auto* popup=field->parentItem();while(popup&&!QString(popup->metaObject()->className()).contains("PopupItem"))popup=popup->parentItem();check(popup!=nullptr,"file dialog surface exists");
                auto bounds=popup->mapRectToScene(QRectF(0,0,popup->width(),popup->height()));auto child=field->mapRectToScene(QRectF(0,0,field->width(),field->height()));
                auto label=QString("%1x%2 scale %3").arg(size.width()).arg(size.height()).arg(scale);
                if(!bounds.contains(child)){
                    window->grabWindow().save(QDir(dir).filePath("dialog-overflow.png"));
                    write(QDir(dir).filePath("dialog-overflow.json"),QJsonDocument(QJsonObject{{"size",label},{"popupX",bounds.x()},{"popupY",bounds.y()},{"popupWidth",bounds.width()},{"popupHeight",bounds.height()},{"fieldX",child.x()},{"fieldY",child.y()},{"fieldWidth",child.width()},{"fieldHeight",child.height()}}).toJson());
                }
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
        click("moreButton");click("findReplaceMenuItem");check(searchBar->isVisible(),"More menu opens find and replace");
        doc->setText("cat catapult CAT");QTest::qWait(30);editor->setProperty("cursorPosition",0);
        findItem(window->contentItem(),"searchField")->setProperty("text","cat");findItem(window->contentItem(),"replaceField")->setProperty("text","dog");click("findNext");
        check(editor->property("selectedText")=="cat","Find Next button selects a match in the current editor");click("replaceOne");check(doc->text().startsWith("dog "),"Replace button updates the actual document");
        editor->forceActiveFocus();QTest::keySequence(window,QKeySequence::Undo);QTest::qWait(20);check(doc->text()=="cat catapult CAT","single replacement is one undo operation");
        click("replaceAll");check(doc->text()=="dog dogapult dog","Replace All button changes every matching occurrence");click("closeSearch");check(!searchBar->isVisible(),"find and replace closes without hiding the editor");
        auto* toolbar=findItem(window->contentItem(),"appToolbar");toolbar->setProperty("safeTop",48);QTest::qWait(40);
        check(findItem(window->contentItem(),"filesButton")->mapToScene(QPointF(0,0)).y()>=48,"toolbar controls respect a top system inset");
        click("filesButton");auto* openWorkspace=findItem(window->contentItem(),"openWorkspaceMenuItem");
        check(openWorkspace&&openWorkspace->isVisible()&&openWorkspace->mapToScene(QPointF(0,0)).y()>=toolbar->property("height").toReal(),"Files menu opens below the safe-area header");
        click("openWorkspaceMenuItem");auto* folderDialog=window->findChild<QObject*>("workspaceFolderDialog");
        check(folderDialog&&folderDialog->property("visible").toBool(),"Open workspace menu click reaches the folder chooser");QMetaObject::invokeMethod(folderDialog,"reject");QTest::qWait(250);
        click("moreButton");auto* settingsItem=findItem(window->contentItem(),"settingsMenuItem");
        check(settingsItem&&settingsItem->isVisible()&&settingsItem->mapToScene(QPointF(0,0)).y()>=toolbar->property("height").toReal(),"More menu remains below the safe-area header");
        QMetaObject::invokeMethod(window->findChild<QObject*>("moreMenu"),"close");toolbar->setProperty("safeTop",0);QTest::qWait(250);window->requestActivate();
        doc->setText("let a = 1;\nlet aa = 2;\n// a\n");QTest::qWait(30);
        check(services->replaceAll(textDocument,"a","counter",true,true)==2&&doc->text().contains("let aa"),"whole-word replace all uses document model");
        editor->forceActiveFocus();QTest::keySequence(window,QKeySequence(QKeySequence::Undo));QTest::qWait(20);check(doc->text()=="let a = 1;\nlet aa = 2;\n// a\n","replace all is one undo operation");
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
        QTemporaryDir copyFixtures(QDir(dir).filePath("copy-XXXXXX"));check(copyFixtures.isValid(),"isolated workspace-copy fixture available");
        auto sourceCopy=copyFixtures.path()+"/source";auto targetCopies=copyFixtures.path()+"/targets";QDir().mkpath(sourceCopy+"/nested");QDir().mkpath(targetCopies);
        write(sourceCopy+"/nested/program.vm","push constant 7\r\n");write(sourceCopy+"/.config.txt","keep hidden configuration");
        auto copied=storage::copyWorkspace(QUrl::fromLocalFile(sourceCopy),QUrl::fromLocalFile(targetCopies),"editable-copy");
        check(bytes(copied.toLocalFile()+"/nested/program.vm")=="push constant 7\r\n"&&bytes(copied.toLocalFile()+"/.config.txt")=="keep hidden configuration","workspace copy preserves nested file bytes and hidden configuration");
        write(copied.toLocalFile()+"/nested/program.vm","local change");check(bytes(sourceCopy+"/nested/program.vm")=="push constant 7\r\n","editing imported copy preserves original project");
        bool copyRejected=false;try{storage::copyWorkspace(QUrl::fromLocalFile(sourceCopy),QUrl::fromLocalFile(targetCopies),"editable-copy");}catch(const std::exception&){copyRejected=true;}check(copyRejected&&bytes(copied.toLocalFile()+"/nested/program.vm")=="local change","copy refuses to replace an existing destination");
        copyRejected=false;try{storage::copyWorkspace(QUrl::fromLocalFile(sourceCopy),QUrl::fromLocalFile(sourceCopy),"recursive");}catch(const std::exception&){copyRejected=true;}check(copyRejected,"copy rejects a destination inside its source");
        copyRejected=false;try{storage::copyWorkspace(QUrl::fromLocalFile(sourceCopy),QUrl::fromLocalFile(targetCopies),"cancelled",[]{return true;});}catch(const std::exception&){copyRejected=true;}check(copyRejected&&!QFileInfo::exists(targetCopies+"/cancelled"),"cancelled workspace copy creates no destination");
        auto vmPath=sourceCopy+"/Flow.vm";
        write(vmPath,"function Sys.init 0\npush constant 7\ncall Foo.double 1\npop temp 0\nlabel END\ngoto END\nfunction Foo.double 0\npush argument 0\npush argument 0\nadd\nreturn\n");
        studio.open(QUrl::fromLocalFile(vmPath));studio.loadVm();studio.step(3);wait();
        auto recoveryPath=qEnvironmentVariable("NAND_TEST_STATE_DIR")+"/recovery.json";
        auto recovery=QJsonDocument::fromJson(bytes(recoveryPath)).object();
        check(recovery["documents"].toArray().at(recovery["active"].toInt()).toObject()["path"]==vmPath,"active document is persisted immediately for process recreation");
        auto vmView=studio.vmInspection();check(vmView["calls"].toList()==QVariantList{QString("Foo.double")}&&vmView["instruction"]=="function Foo.double 0","VM inspector records actual call and current instruction");
        check(vmView["segments"].toList()[1].toMap()["value"]==262&&!vmView["stack"].toList().isEmpty(),"VM inspector exposes frame pointers and RAM stack words");
        studio.step(5);wait();vmView=studio.vmInspection();check(vmView["calls"].toList().isEmpty()&&vmView["instruction"]=="pop temp 0"&&vmView["stack"].toList().last().toMap()["value"]==14,"VM return updates visible calls and stack result");
        studio.setBreakpoint(5,true);studio.step(100);wait();check(studio.state()["PC"]==5&&studio.memory(5)==14,"VM run stops at PC breakpoint after publishing returned value");studio.setBreakpoint(5,false);
        check(findItem(window->contentItem(),"vmInstruction")!=nullptr,"VM instruction inspector is present in the shared responsive UI");
        auto* vmDocument=qvariant_cast<Document*>(studio.documents()[studio.active()]);vmDocument->setText(vmDocument->text()+"// recovery marker\n");QTest::qWait(350);
        recovery=QJsonDocument::fromJson(bytes(recoveryPath)).object();check(recovery["documents"].toArray().at(recovery["active"].toInt()).toObject()["text"].toString().endsWith("// recovery marker\n"),"edited buffer is journaled shortly after typing");
        check(vmDocument->save(),"VM recovery fixture saved");studio.closeDocument(studio.active());studio.open(QUrl::fromLocalFile(path));
        auto runtimeLog=bytes(qEnvironmentVariable("NAND_TEST_STATE_DIR")+"/qt.log");runtimeLog=runtimeLog.mid(runtimeLog.lastIndexOf("Creating application"));
        check(!runtimeLog.contains("Binding loop")&&!runtimeLog.contains("TypeError")&&!runtimeLog.contains("ReferenceError")&&!runtimeLog.contains("QDataStream::operator"),"no runtime QML binding, type, or settings serialization errors");
    }catch(const std::exception& e){qWarning("GUI check failed: %s",e.what());exitCode=1;}
    write(QDir(dir).filePath("checks.json"),QJsonDocument(checks).toJson());QCoreApplication::exit(exitCode);
}
