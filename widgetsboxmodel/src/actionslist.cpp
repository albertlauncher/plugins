// Copyright (c) 2022 Manuel Schneider

#include <QKeyEvent>
#include <QPainter>
#include "actionslist.h"

ActionsList::ActionsList(QWidget *parent) : ResizingList(parent)
{
    setItemDelegate(new ActionDelegate);
}

void ActionsList::ActionDelegate::paint(QPainter *painter, const QStyleOptionViewItem &options, const QModelIndex &index) const
{
    painter->save();

    QStyleOptionViewItem option = options;
    initStyleOption(&option, index);

    // Draw selection
    option.widget->style()->drawPrimitive(QStyle::PE_PanelItemViewItem, &option, painter, option.widget);

    // Draw text
    painter->setFont(option.font);
    QString text = QFontMetrics(option.font).elidedText(index.data(Qt::DisplayRole).toString(), option.textElideMode, option.rect.width());
    option.widget->style()->drawItemText(painter,
                                         option.rect,
                                         Qt::AlignCenter|Qt::AlignHCenter,
                                         option.palette,
                                         option.state & QStyle::State_Enabled,
                                         text,
                                         (option.state & QStyle::State_Selected) ? QPalette::HighlightedText : QPalette::WindowText);
    painter->restore();
}
