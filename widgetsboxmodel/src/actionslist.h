// Copyright (c) 2014-2024 Manuel Schneider

#pragma once
#include "resizinglist.h"
class ActionDelegate;

class ActionsList : public ResizingList
{
public:
    ActionsList(QWidget *parent = nullptr);
    void setStyle(const Style*);
private:
    ActionDelegate *delegate;
};
