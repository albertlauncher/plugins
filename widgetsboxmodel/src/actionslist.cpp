// Copyright (c) 2014-2024 Manuel Schneider

#include "actionslist.h"
#include "itemdelegatebase.h"
#include "primitives.h"
#include "style.h"
#include <QPainter>
#include <albert/logging.h>

class ActionDelegate : public ItemDelegateBase
{
public:
    using ItemDelegateBase::ItemDelegateBase;
protected:

    QSize sizeHint(const QStyleOptionViewItem &o, const QModelIndex&) const override
    {
        return {o.widget->width(), o.fontMetrics.height() + 2 * style_->item_view_item_padding};
    }

    void paint(QPainter *p, const QStyleOptionViewItem &o, const QModelIndex &i) const override
    {
        DEBG << "Painting action item";
        // QStyleOptionViewItem o = options;
        // initStyleOption(&o, index);

        // Draw selection
        ItemDelegateBase::paint(p, o, i); // background
        if(o.state.testFlag(QStyle::State_Selected))
        {
            QPen pen(o.widget->palette().highlightedText(), 2);
            p->setPen(pen);
        }


        // Draw selection
        // ItemDelegateBase::paint(painter, o, index);
        // o.state |= QStyle::State_Enabled| QStyle::State_HasFocus;
        // o.widget->style()->drawPrimitive(QStyle::PE_PanelItemViewItem, &o, p, o.widget);

        // Draw text
        QString text = QFontMetrics(o.font).elidedText(i.data(Qt::DisplayRole).toString(), o.textElideMode, o.rect.width());

        auto f = o.font;
        f.setPointSize(style_->action_item_font_size);
        p->setFont(f);
        p->setPen(QPen(style_->action_item_text_color, 0));
        o.widget->style()->drawItemText(p,
                                        o.rect,
                                        Qt::AlignCenter,
                                        o.palette,
                                        o.state & QStyle::State_Enabled,
                                        text,
                                        (o.state & QStyle::State_Selected) ? QPalette::HighlightedText : QPalette::WindowText);

        if (style_->draw_debug_overlays){
            drawDebugRect(*p, o.rect, "ActionDelegate::rect");
        }
    }
};

ActionsList::ActionsList(QWidget *parent):
    ResizingList(parent),
    delegate(new ActionDelegate(this))
{
    setItemDelegate(delegate);
}

void ActionsList::setStyle(const Style *s)
{
    ResizingList::setStyle(s);
    delegate->setStyle(s);

    auto f = font();
    f.setPointSize(style->action_item_font_size);
    setFont(f);
}
