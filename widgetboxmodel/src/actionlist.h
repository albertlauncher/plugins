// Copyright (C) 2014-2018 Manuel Schneider

#pragma once
#include <QEvent>
#include "resizinglist.h"
#include <QStyledItemDelegate>

namespace WidgetBoxModel {


/** ***************************************************************************/
class ActionList final : public ResizingList
{
    Q_OBJECT
    class ActionDelegate;

public:

    ActionList(QWidget *parent = 0);

private:

    bool eventFilter(QObject*, QEvent *event) override;

};



/** ***************************************************************************/
class ActionList::ActionDelegate final : public QStyledItemDelegate
{
public:

    ActionDelegate(QObject *parent = nullptr) : QStyledItemDelegate(parent) {}

    void paint(QPainter *painter, const QStyleOptionViewItem &options, const QModelIndex &index) const override;
};

}
