// Copyright (c) 2014-2025 Manuel Schneider

#include "actionslist.h"
#include "primitives.h"
#include <QPainter>
#include <albert/logging.h>


class ActionsListDelegate : public ItemDelegateBase
{
public:

    QSize sizeHint(const QStyleOptionViewItem &o, const QModelIndex&) const override;
    void paint(QPainter *p, const QStyleOptionViewItem &o, const QModelIndex &i) const override;

};

//--------------------------------------------------------------------------------------------------

ActionsList::ActionsList(QWidget *parent) : ResizingList(parent)
{
    delegate_ = new ActionsListDelegate;
    setItemDelegate(delegate_);
}

ActionsList::~ActionsList() { delete delegate_; }

ItemDelegateBase *ActionsList::delegate() const { return delegate_; }

//--------------------------------------------------------------------------------------------------

QSize ActionsListDelegate::sizeHint(const QStyleOptionViewItem &o, const QModelIndex &) const
{
    return { o.widget->width(), o.fontMetrics.height() + 2 * padding };
}

void ActionsListDelegate::paint(QPainter *p, const QStyleOptionViewItem &o, const QModelIndex &i) const
{
    p->save();

    auto selected = o.state.testFlag(QStyle::State_Selected);

    // Draw selection
    ItemDelegateBase::paint(p, o, i);

    // Elide text
    auto text = i.data(Qt::DisplayRole).toString();
    text = p->fontMetrics().elidedText(text, o.textElideMode, o.rect.width());

    QTextOption text_option;
    text_option.setAlignment(Qt::AlignCenter);
    p->setFont(text_font);
    p->setPen(QPen(selected ? selection_text_color : text_color, 0));
    p->drawText(o.rect, text, text_option);


    // Draw debug rect
    if (draw_debug_overlays)
        drawDebugRect(*p, o.rect, "ActionDelegate::rect");

    p->restore();
}
