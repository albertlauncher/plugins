// Copyright (c) 2014-2023 Manuel Schneider

#pragma once
#include "albert/util/iconprovider.h"
#include <QStyledItemDelegate>

class ItemDelegate : public QStyledItemDelegate
{
public:
    ItemDelegate(QObject *parent = nullptr);

private:
    void paint(QPainter *painter, const QStyleOptionViewItem &options, const QModelIndex &index) const override;
    albert::IconProvider icon_provider;
};
