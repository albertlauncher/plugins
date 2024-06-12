// Copyright (c) 2014-2024 Manuel Schneider

#pragma once
#include <QStyledItemDelegate>
class Style;

class ItemDelegateBase : public QStyledItemDelegate
{
public:
    using QStyledItemDelegate::QStyledItemDelegate;
    void setStyle(const Style*);

protected:
    void paint(QPainter *painter, const QStyleOptionViewItem &options, const QModelIndex &index) const override;
    const Style *style_;

};
