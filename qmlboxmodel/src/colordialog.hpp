// Copyright (C) 2014-2018 Manuel Schneider

#pragma once
#include <QColorDialog>

namespace QmlBoxModel {

class ColorDialog : public QColorDialog
{
    Q_OBJECT
    Q_PROPERTY(QColor color READ color WRITE setColor NOTIFY colorChanged USER true)
public:
    ColorDialog(QWidget * parent = 0) : QColorDialog(parent){
        setOptions(QColorDialog::ShowAlphaChannel);
        connect(this, &QColorDialog::currentColorChanged,
                this, &ColorDialog::colorChanged);
    }

    QColor color(){ return currentColor(); }
    void setColor(const QColor& c){ setCurrentColor(c); }
signals:
    void colorChanged(const QColor &color);
};

}
