// Copyright (c) 2014-2025 Manuel Schneider

#pragma once
#include <QStyledItemDelegate>

class ItemDelegateBase : public QStyledItemDelegate
{
public:

    QBrush selection_background_brush;
    QBrush selection_border_brush;
    double selection_border_radius;
    double selection_border_width;
    int padding;

protected:

    void paint(QPainter *painter, const QStyleOptionViewItem &options, const QModelIndex &index) const override;

};
