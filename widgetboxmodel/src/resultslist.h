// Copyright (C) 2014-2018 Manuel Schneider

#pragma once
#include <QEvent>
#include "resizinglist.h"
#include <QStyledItemDelegate>

namespace WidgetBoxModel {

class ResultsList final : public ResizingList
{
    Q_OBJECT
    class ItemDelegate;

public:

    ResultsList(QWidget *parent = 0);

    bool displayIcons() const;
    void setDisplayIcons(bool value);

private:

    bool eventFilter(QObject*, QEvent *event) override;
    void showEvent(QShowEvent *event) override;

    ItemDelegate *delegate_;
};
}
