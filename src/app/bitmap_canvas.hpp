// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include "bitmap.hpp"
#include <QQuickPaintedItem>
#include <QPainter>
#include <QGuiApplication>
#include <QClipboard>
class BitmapCanvas : public QQuickPaintedItem {
    Q_OBJECT
    Q_PROPERTY(int columns READ columns NOTIFY changed)
    Q_PROPERTY(int rows READ rows NOTIFY changed)
    Q_PROPERTY(QString jack READ jack NOTIFY changed)
    Q_PROPERTY(QString assembly READ assembly NOTIFY changed)
    Q_PROPERTY(bool canUndo READ canUndo NOTIFY changed)
    Q_PROPERTY(bool canRedo READ canRedo NOTIFY changed)
    nand::Bitmap bitmap_;std::vector<nand::Bitmap> undo_,redo_;
    bool painting_=false,ink_=true;
    void checkpoint(){if(undo_.size()==32)undo_.erase(undo_.begin());undo_.push_back(bitmap_);redo_.clear();}
    void refresh(){update();emit changed();}
public:
    using QQuickPaintedItem::QQuickPaintedItem;
    int columns()const{return bitmap_.width();}int rows()const{return bitmap_.height();}
    QString jack()const{return QString::fromStdString(bitmap_.jack());}
    QString assembly()const{return QString::fromStdString(bitmap_.assembly());}
    bool canUndo()const{return !undo_.empty();}bool canRedo()const{return !redo_.empty();}
    Q_INVOKABLE bool resizeCanvas(int w,int h){try{auto next=bitmap_;next.resize(w,h);checkpoint();bitmap_=std::move(next);refresh();return true;}catch(...){return false;}}
    Q_INVOKABLE void beginStroke(int x,int y){if(x<0||y<0||x>=columns()||y>=rows())return;checkpoint();ink_=!bitmap_.pixel(x,y);painting_=true;stroke(x,y);}
    Q_INVOKABLE void stroke(int x,int y){if(painting_){bitmap_.set(x,y,ink_);update();}}
    Q_INVOKABLE void endStroke(){if(painting_){painting_=false;refresh();}}
    Q_INVOKABLE void transform(const QString& op){auto next=bitmap_;if(op=="clear")next.clear();else if(op=="invert")next.invert();else if(op=="flip")next.flip();else if(op=="left")next.shift(-1,0);else if(op=="right")next.shift(1,0);else if(op=="up")next.shift(0,-1);else if(op=="down")next.shift(0,1);else if(op=="rotate"&&columns()==rows())next.rotate();else return;checkpoint();bitmap_=std::move(next);refresh();}
    Q_INVOKABLE void undo(){if(undo_.empty())return;redo_.push_back(bitmap_);bitmap_=undo_.back();undo_.pop_back();refresh();}
    Q_INVOKABLE void redo(){if(redo_.empty())return;undo_.push_back(bitmap_);bitmap_=redo_.back();redo_.pop_back();refresh();}
    Q_INVOKABLE void copyCode(bool asmCode){QGuiApplication::clipboard()->setText(asmCode?assembly():jack());}
    void paint(QPainter* p)override{p->fillRect(boundingRect(),Qt::white);const auto dx=width()/columns(),dy=height()/rows();for(int y=0;y<rows();++y)for(int x=0;x<columns();++x)if(bitmap_.pixel(x,y))p->fillRect(QRectF(x*dx,y*dy,dx,dy),Qt::black);p->setPen(QColor("#b0b0b0"));if(dx>=6&&dy>=6){for(int x=0;x<=columns();++x)p->drawLine(QPointF(x*dx,0),QPointF(x*dx,height()));for(int y=0;y<=rows();++y)p->drawLine(QPointF(0,y*dy),QPointF(width(),y*dy));}}
signals:void changed();
};
