// Copyright (c) 2014-2023 Manuel Schneider

#pragma once
#include <QStyledItemDelegate>

class ActionDelegate : public QStyledItemDelegate
{
public:
    explicit ActionDelegate(QObject *parent = nullptr);

private:
    void paint(QPainter *painter, const QStyleOptionViewItem &options, const QModelIndex &index) const override;
};

