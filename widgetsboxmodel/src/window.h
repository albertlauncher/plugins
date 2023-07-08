// Copyright (c) 2022 Manuel Schneider

#pragma once
#include <QPoint>
#include <QWidget>
class ActionDelegate;
class InputLine;
class ItemDelegate;
class QEvent;
class QFrame;
class ResizingList;
class SettingsButton;

class Window : public QWidget
{
public:
    Window();

    QFrame *frame;
    InputLine *input_line;
    SettingsButton *settings_button;
    ResizingList *results_list;
    ResizingList *actions_list;
    ItemDelegate *item_delegate;
    ActionDelegate *action_delegate;

private:
    bool event(QEvent *event) override;

    QPoint clickOffset_;  // The offset from cursor to topleft. Used when the window is dagged

};
