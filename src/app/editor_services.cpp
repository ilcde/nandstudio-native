// SPDX-License-Identifier: GPL-3.0-or-later
#include "editor_services.hpp"
#include <QTextCursor>
#include <QTextBlock>
#include <QTextLayout>
#include <QPainter>
#include <QJSValue>
#include <QDir>
#include <QStandardPaths>
#include <algorithm>
namespace {
QString settingsFile(){auto dir=qEnvironmentVariable("NAND_TEST_STATE_DIR");if(dir.isEmpty())dir=QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation);QDir().mkpath(dir);return dir+"/preferences.ini";}
QTextDocument* doc(QQuickTextDocument* d){return d?d->textDocument():nullptr;}
QVariantMap editLines(QTextDocument* d,int start,int end,const std::function<QString(QString)>& change){
    if(!d)return {};start=std::clamp(start,0,d->characterCount()-1);end=std::clamp(end,start,d->characterCount()-1);
    auto first=d->findBlock(start),last=d->findBlock(end>start?end-1:end);int a=first.position(),b=last.position()+last.length()-1;
    QStringList result;for(auto block=first;block.isValid();block=block.next()){result.append(change(block.text()));if(block==last)break;}
    QTextCursor cursor(d);cursor.beginEditBlock();cursor.setPosition(a);cursor.setPosition(b,QTextCursor::KeepAnchor);auto replacement=result.join('\n');cursor.insertText(replacement);cursor.endEditBlock();return {{"start",a},{"end",a+replacement.size()}};
}
bool wordCharacter(QChar c){return c.isLetterOrNumber()||c=='_';}
}
Preferences::Preferences():settings_(settingsFile(),QSettings::IniFormat){
    values_={{"dark",true},{"fontSize",15},{"fontFamily","monospace"},{"indentWidth",4},{"indentTabs",false},{"wordWrap",false},{"uiScale",1.0},{"autosaveSeconds",0},{"workspaceWidth",240},{"machineWidth",350},{"consoleHeight",220},{"executionBatch",1000},{"recentWorkspaces",QStringList{}}};
    for(auto i=values_.begin();i!=values_.end();++i){auto saved=settings_.value(i.key(),i.value());if(saved.isValid()&&saved.convert(i.value().metaType()))i.value()=saved;}
}
void Preferences::set(const QString& name,const QVariant& input){
    if(!values_.contains(name))return;
    auto value=input.metaType()==QMetaType::fromType<QJSValue>()?input.value<QJSValue>().toVariant():input;
    if(!value.convert(values_[name].metaType()))return;
    if(name=="fontSize")value=std::clamp(value.toInt(),10,32);
    else if(name=="indentWidth")value=std::clamp(value.toInt(),1,8);
    else if(name=="uiScale")value=std::clamp(value.toDouble(),1.0,1.5);
    else if(name=="autosaveSeconds")value=std::clamp(value.toInt(),0,600);
    if(values_[name]==value)return;
    values_[name]=value;settings_.setValue(name,value);settings_.sync();emit changed();
}
QVariantMap EditorServices::indent(QQuickTextDocument* q,int start,int end,int width,bool tabs,bool remove){width=std::clamp(width,1,8);return editLines(doc(q),start,end,[=](QString line){if(!remove)return (tabs?QString("\t"):QString(width,' '))+line;if(line.startsWith('\t'))return line.mid(1);int n=0;while(n<width&&n<line.size()&&line[n]==' ')++n;return line.mid(n);});}
QVariantMap EditorServices::comment(QQuickTextDocument* q,int start,int end){auto* d=doc(q);if(!d)return {};bool uncomment=true;auto last=d->findBlock(end>start?end-1:end);for(auto b=d->findBlock(start);b.isValid();b=b.next()){if(!b.text().trimmed().startsWith("//"))uncomment=false;if(b==last)break;}return editLines(d,start,end,[=](QString line){int n=0;while(n<line.size()&&line[n].isSpace())++n;if(uncomment)line.remove(n,line.mid(n,3)=="// "?3:2);else line.insert(n,"// ");return line;});}
int EditorServices::newline(QQuickTextDocument* q,int position,int width,bool tabs){auto* d=doc(q);if(!d)return position;QTextCursor cursor(d);cursor.setPosition(std::clamp(position,0,d->characterCount()-1));auto before=cursor.block().text().left(cursor.positionInBlock());QString indentation;for(auto c:before){if(c!=' '&&c!='\t')break;indentation+=c;}if(before.trimmed().endsWith('{'))indentation+=tabs?QString("\t"):QString(std::clamp(width,1,8),' ');cursor.beginEditBlock();cursor.insertText("\n"+indentation);cursor.endEditBlock();return cursor.position();}
QVariantMap EditorServices::find(const QString& text,const QString& query,int from,bool sensitive,bool word)const{
    if(query.isEmpty())return {{"start",-1},{"end",-1}};auto cs=sensitive?Qt::CaseSensitive:Qt::CaseInsensitive;
    auto search=[&](int start,int stop){int p=std::max(start,0);while((p=int(text.indexOf(query,p,cs)))>=0&&p<stop){int end=p+int(query.size());if(!word||((p==0||!wordCharacter(text[p-1]))&&(end==text.size()||!wordCharacter(text[end]))))return p;++p;}return -1;};
    int p=search(from,int(text.size()));if(p<0)p=search(0,std::clamp(from,0,int(text.size())));return {{"start",p},{"end",p<0?-1:p+int(query.size())}};
}
int EditorServices::replaceAll(QQuickTextDocument* q,const QString& query,const QString& replacement,bool sensitive,bool word){auto* d=doc(q);if(!d||query.isEmpty())return 0;auto text=d->toPlainText();QList<int> positions;int from=0;while(from<text.size()){auto m=find(text,query,from,sensitive,word);int p=m["start"].toInt();if(p<from)break;positions.append(p);from=m["end"].toInt();}QTextCursor c(d);c.beginEditBlock();for(auto i=positions.crbegin();i!=positions.crend();++i){c.setPosition(*i);c.setPosition(*i+int(query.size()),QTextCursor::KeepAnchor);c.insertText(replacement);}c.endEditBlock();return int(positions.size());}
int EditorServices::matchingBracket(const QString& text,int cursor)const{
    const QString open="([{",close=")]}",all=open+close;int at=cursor>0&&cursor<=text.size()&&all.contains(text.at(cursor-1))?cursor-1:cursor;if(at<0||at>=text.size()||!all.contains(text[at]))return -1;
    QList<QPair<QChar,int>> stack;bool string=false,line=false,block=false,escape=false;
    for(int i=0;i<text.size();++i){auto c=text[i],next=i+1<text.size()?text[i+1]:QChar();if(line){if(c=='\n')line=false;continue;}if(block){if(c=='*'&&next=='/'){block=false;++i;}continue;}if(string){if(c=='"'&&!escape)string=false;escape=c=='\\'&&!escape;continue;}if(c=='/'&&next=='/'){line=true;++i;continue;}if(c=='/'&&next=='*'){block=true;++i;continue;}if(c=='"'){string=true;continue;}if(open.contains(c))stack.append({c,i});else if(close.contains(c)){if(stack.isEmpty()||open.indexOf(stack.last().first)!=close.indexOf(c)){stack.clear();continue;}auto a=stack.takeLast();if(a.second==at)return i;if(i==at)return a.second;}}
    return -1;
}
LineNumberGutter::LineNumberGutter(QQuickItem* parent):QQuickPaintedItem(parent){connect(this,&LineNumberGutter::geometryChanged,this,[this]{update();});}
void LineNumberGutter::setDocument(QQuickTextDocument* d){if(document_==d)return;if(document_&&document_->textDocument())disconnect(document_->textDocument(),nullptr,this,nullptr);document_=d;if(d)connect(d->textDocument(),&QTextDocument::contentsChanged,this,[this]{update();});emit documentChanged();update();}
void LineNumberGutter::paint(QPainter* painter){if(!document_)return;painter->setFont(font_);painter->setPen(color_);int n=1;for(auto block=document_->textDocument()->begin();block.isValid();block=block.next(),++n){auto* layout=block.layout();if(!layout||layout->lineCount()==0)continue;auto line=layout->lineAt(0);qreal y=layout->position().y()+line.y()+topPadding_-scrollY_;if(y+line.height()<0)continue;if(y>height())break;painter->drawText(QRectF(0,y,width()-10,line.height()),Qt::AlignRight|Qt::AlignVCenter,QString::number(n));}}
