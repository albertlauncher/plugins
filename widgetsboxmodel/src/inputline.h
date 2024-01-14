// Copyright (c) 2022-2024 Manuel Schneider

#pragma once
#include "albert/extension/frontend/inputhistory.h"
#include <QLineEdit>

// Input method problems: https://bugreports.qt.io/browse/QTBUG-106516
class InputLine : public QLineEdit
{
public:

    InputLine(QWidget *parent);

    void setInputHint(const QString &text);
    void setTriggerLength(uint len);

    void next(bool search);
    void previous(bool search);

private:

    void paintEvent(QPaintEvent *event) override;
    void hideEvent(QHideEvent *event) override;

    QString input_hint_;
    albert::InputHistory history_;
    QString user_text_; // used for history search
    uint trigger_length_;

};
