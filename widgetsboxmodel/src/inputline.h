// Copyright (c) 2022 Manuel Schneider

#pragma once
#include <QLineEdit>
#include "settingsbutton.h"


// Input method problems: https://bugreports.qt.io/browse/QTBUG-106516
class InputLine : public QLineEdit
{
public:
    explicit InputLine(QWidget *parent);
    void paintEvent(QPaintEvent *event) override;
    void setInputHint(const QString &text);

private:
    QString input_hint;


};
