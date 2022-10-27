// Copyright (c) 2022 Manuel Schneider

#pragma once
#include <QTimer>
#include <QWidget>
#include <map>
#include <memory>
#include <QFrame>
#include "inputline.h"
#include "settingsbutton.h"
#include "itemslist.h"
#include "actionslist.h"

class Window : public QWidget
{
public:
    Window();

    QFrame *frame;
    InputLine *input_line;
    SettingsButton *settings_button;
    ItemsList *results_list;
    ActionsList *actions_list;

private:
    bool event(QEvent *event) override;

    QPoint clickOffset_;  // The offset from cursor to topleft. Used when the window is dagged

};
