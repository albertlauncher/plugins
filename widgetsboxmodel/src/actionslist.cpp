// Copyright (c) 2014-2025 Manuel Schneider

#include "actionslist.h"
#include "itemdelegatebase.h"
#include "primitives.h"
#include <QPainter>
#include <albert/logging.h>


class ActionsListDelegate : public ItemDelegateBase
{
public:

    ActionsListDelegate();

    QColor text_color;
    bool draw_debug_overlays;

    QSize sizeHint(const QStyleOptionViewItem &o, const QModelIndex&) const override;
    void paint(QPainter *p, const QStyleOptionViewItem &o, const QModelIndex &i) const override;

};

//--------------------------------------------------------------------------------------------------

ActionsList::ActionsList(QWidget *parent) : ResizingList(parent)
{
    delegate_ = new ActionsListDelegate;
    setItemDelegate(delegate_);
}

ActionsList::~ActionsList()
{
    delete delegate_;
}

ItemDelegateBase *ActionsList::delegate() const { return delegate_; }

uint ActionsList::fontSize() const { return font().pointSize(); }

void ActionsList::setFontSize(uint val)
{
    auto f = font();
    f.setPointSize(val);
    setFont(f);
    relayout();
}

QColor ActionsList::textColor() const { return delegate_->text_color; }

void ActionsList::setTextColor(QColor val) { delegate_->text_color = val; update(); }

bool ActionsList::debugMode() const { return delegate_->draw_debug_overlays; }

void ActionsList::setDebugMode(bool val) { delegate_->draw_debug_overlays = val; update(); }

//--------------------------------------------------------------------------------------------------

ActionsListDelegate::ActionsListDelegate():
    draw_debug_overlays(false)
{

}

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

    // Draw text
    o.widget->style()->drawItemText(p,
                                    o.rect,
                                    Qt::AlignCenter,
                                    o.palette,
                                    o.state & QStyle::State_Enabled,
                                    text,
                                    selected ? QPalette::HighlightedText : QPalette::WindowText);

    // Draw debug rect
    if (draw_debug_overlays)
    {
        drawDebugRect(*p, o.rect, "ActionDelegate::rect");
    }

    p->restore();
}
