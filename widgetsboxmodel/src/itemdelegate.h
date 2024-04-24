// Copyright (c) 2014-2024 Manuel Schneider

#pragma once
#include <QStyledItemDelegate>

class ItemDelegate : public QStyledItemDelegate
{
public:
    ItemDelegate(QObject *parent = nullptr);
private:
    void paint(QPainter *painter, const QStyleOptionViewItem &options, const QModelIndex &index) const override;
};
