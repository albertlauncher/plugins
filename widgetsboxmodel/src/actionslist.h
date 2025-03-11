// Copyright (c) 2014-2025 Manuel Schneider

#pragma once
#include "resizinglist.h"
class ActionsListDelegate;

class ActionsList : public ResizingList
{
public:

    ActionsList(QWidget *parent = nullptr);
    ~ActionsList();

    QColor textColor() const;
    void setTextColor(QColor);

    uint fontSize() const;
    void setFontSize(uint);

    bool debugMode() const;
    void setDebugMode(bool);

private:

    ItemDelegateBase *delegate() const override;
    ActionsListDelegate *delegate_;

};
