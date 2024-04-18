// Copyright (c) 2022-2024 Manuel Schneider

#pragma once
#include <QColorDialog>

class ColorDialog : public QColorDialog
{
    Q_OBJECT
    Q_PROPERTY(QColor color READ color WRITE setColor NOTIFY colorChanged USER true)

public:
    ColorDialog(QWidget * parent = 0);

    QColor color();
    void setColor(const QColor& c);

signals:
    void colorChanged(const QColor &color);
};

