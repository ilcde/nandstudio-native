// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include <QObject>
#include <functional>
#include <QQuickTextDocument>
#include <QQuickPaintedItem>
#include <QSettings>
#include <QVariantMap>
#include <QPointer>
class Preferences : public QObject {
    Q_OBJECT
    Q_PROPERTY(QVariantMap values READ values NOTIFY changed)
public:
    Preferences();
    QVariantMap values()const{return values_;}
    Q_INVOKABLE void set(const QString& name,const QVariant& value);
signals:void changed();
private:QSettings settings_;QVariantMap values_;
};
class EditorServices : public QObject {
    Q_OBJECT
public:using QObject::QObject;
    Q_INVOKABLE QVariantMap indent(QQuickTextDocument*,int start,int end,int width,bool tabs,bool remove);
    Q_INVOKABLE QVariantMap comment(QQuickTextDocument*,int start,int end);
    Q_INVOKABLE int newline(QQuickTextDocument*,int position,int width,bool tabs);
    Q_INVOKABLE QVariantMap find(const QString& text,const QString& query,int from,bool sensitive,bool word)const;
    Q_INVOKABLE int replaceAll(QQuickTextDocument*,const QString& query,const QString& replacement,bool sensitive,bool word);
    Q_INVOKABLE QVariantMap replaceOne(QQuickTextDocument*,const QString& query,const QString& replacement,int from,bool sensitive,bool word);
    Q_INVOKABLE int matchingBracket(const QString& text,int cursor)const;
};
class LineNumberGutter : public QQuickPaintedItem {
    Q_OBJECT
    Q_PROPERTY(QQuickTextDocument* document READ document WRITE setDocument NOTIFY documentChanged)
    Q_PROPERTY(qreal scrollY MEMBER scrollY_ NOTIFY geometryChanged)
    Q_PROPERTY(qreal topPadding MEMBER topPadding_ NOTIFY geometryChanged)
    Q_PROPERTY(QColor textColor MEMBER color_ NOTIFY geometryChanged)
    Q_PROPERTY(QFont textFont MEMBER font_ NOTIFY geometryChanged)
public:
    LineNumberGutter(QQuickItem* parent=nullptr);
    QQuickTextDocument* document()const{return document_;}
    void setDocument(QQuickTextDocument*);
    void paint(QPainter*)override;
signals:void documentChanged();void geometryChanged();
private:QPointer<QQuickTextDocument> document_;qreal scrollY_=0,topPadding_=0;QColor color_;QFont font_;
};
