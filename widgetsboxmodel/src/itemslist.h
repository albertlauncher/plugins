// Copyright (c) 2022 Manuel Schneider

#pragma once
#include <QEvent>
#include "resizinglist.h"
#include <QStyledItemDelegate>

class ItemsList final : public ResizingList
{
    Q_OBJECT
public:
    explicit ItemsList(QWidget *parent = nullptr);
    bool displayIcons() const;
    void setDisplayIcons(bool value);

private:
    class ItemDelegate;
    ItemDelegate *delegate_;
};
