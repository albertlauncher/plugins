// Copyright (c) 2022-2025 Manuel Schneider

#include "primitives.h"
#include "resizinglist.h"
#include <QApplication>
#include <QKeyEvent>
#include <QPainter>
#include <QPixmapCache>
using namespace std;

ItemDelegateBase::ItemDelegateBase():
    text_font(QApplication::font()),
    text_font_metrics(text_font),
    draw_debug_overlays(false)
{

}

void ItemDelegateBase::paint(QPainter *p, const QStyleOptionViewItem &opt, const QModelIndex &) const
{
    if(opt.state.testFlag(QStyle::State_Selected))
    {
        QPixmap pm;
        if (const auto cache_key = QString("_ItemViewSelection_%1x%2")
                                       .arg(opt.rect.width()).arg(opt.rect.height());
            !QPixmapCache::find(cache_key, &pm))
        {
            auto dpr = opt.widget->devicePixelRatioF();
            pm = pixelPerfectRoundedRect(opt.rect.size() * dpr,
                                         selection_background_brush,
                                         (int)(selection_border_radius * dpr),
                                         selection_border_brush,
                                         (int)(selection_border_width * dpr));
            pm.setDevicePixelRatio(dpr);
            QPixmapCache::insert(cache_key, pm);
        }
        p->drawPixmap(opt.rect, pm);
    }
}

ResizingList::ResizingList(QWidget *parent) : QListView(parent)
{
    connect(this, &ResizingList::clicked, this, &ResizingList::activated);

    setFrameShape(QFrame::NoFrame);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
//    setLayoutMode(LayoutMode::Batched);
    setUniformItemSizes(true);
    viewport()->setAutoFillBackground(false);
    hide();
}

void ResizingList::relayout()
{
    updateGeometry();
    reset(); // needed to relayout items
}

bool ResizingList::debugMode() const { return delegate()->draw_debug_overlays; }

void ResizingList::setDebugMode(bool val) { delegate()->draw_debug_overlays = val; update(); }

uint ResizingList::textFontSize() const { return delegate()->text_font.pointSize(); }

void ResizingList::setTextFontSize(uint v)
{
    delegate()->text_font.setPointSize(v);
    delegate()->text_font_metrics = QFontMetrics(delegate()->text_font);
    relayout();
}

QColor ResizingList::textColor() const { return delegate()->text_color; }

void ResizingList::setTextColor(QColor v) { delegate()->text_color = v; update(); }

QColor ResizingList::selectionTextColor() const { return delegate()->selection_text_color; }

void ResizingList::setSelectionTextColor(QColor v) { delegate()->selection_text_color = v; update(); }

QBrush ResizingList::selectionBackgroundBrush() const { return delegate()->selection_background_brush; }

void ResizingList::setSelectionBackgroundBrush(QBrush val) { delegate()->selection_background_brush = val; update(); }

QBrush ResizingList::selectionBorderBrush() const { return delegate()->selection_border_brush; }

void ResizingList::setSelectionBorderBrush(QBrush val) { delegate()->selection_border_brush = val; update(); }

double ResizingList::borderRadius() const { return delegate()->selection_border_radius; }

void ResizingList::setBorderRadius(double val) { delegate()->selection_border_radius = val; update(); }

double ResizingList::borderWidth() const { return delegate()->selection_border_width; }

void ResizingList::setBorderWidth(double val) { delegate()->selection_border_width = val; update(); }

uint ResizingList::padding() const { return delegate()->padding; }

void ResizingList::setPadding(uint val) { delegate()->padding = val; relayout(); }

uint ResizingList::maxItems() const { return maxItems_; }

void ResizingList::setMaxItems(uint maxItems)
{
    maxItems_ = maxItems;
    updateGeometry();
}

QSize ResizingList::sizeHint() const
{
    if (model() == nullptr)
        return {};
    return {width(),
            contentsMargins().bottom() + contentsMargins().top()
                + sizeHintForRow(0) * min(static_cast<int>(maxItems_), model()->rowCount(rootIndex()))};
}

QSize ResizingList::minimumSizeHint() const { return {0,0}; }

void ResizingList::setModel(QAbstractItemModel *m)
{
    if (model() != nullptr)
        disconnect(model(), nullptr, this, nullptr);

    QAbstractItemView::setModel(m);

    if (m != nullptr)
    {
        current_row_count_ = m->rowCount();
        connect(m, &QAbstractItemModel::rowsInserted, this, &ResizingList::onUpdateSelectionAndSize);
        connect(m, &QAbstractItemModel::modelReset, this, &ResizingList::onUpdateSelectionAndSize);
        onUpdateSelectionAndSize();
    }
    else
        current_row_count_ = 0;

    updateGeometry();
}

bool ResizingList::eventFilter(QObject*, QEvent *event)
{
    if (event->type() == QEvent::KeyPress) {
        auto* keyEvent = static_cast<QKeyEvent*>(event);
        switch (keyEvent->key()) {

            // Navigation
            case Qt::Key_Up:
            case Qt::Key_Down:
            case Qt::Key_PageUp:
            case Qt::Key_PageDown:
            // case Qt::Key_Home:  Interferes with the inputline
            // case Qt::Key_End:
                return QListView::event(event);

            case Qt::Key_O:
                if (keyEvent->modifiers().testFlag(Qt::ControlModifier)){
                    emit activated(currentIndex());
                    return true;
                }
                break;

            case Qt::Key_Enter:
            case Qt::Key_Return:
                emit activated(currentIndex());
                return true;
        }
    }
    return false;
}

void ResizingList::onUpdateSelectionAndSize()
{
    // Trigger an update if the addes rows should increase the list size
    if (current_row_count_ < maxItems_)
        updateGeometry();

    current_row_count_ = model()->rowCount();

    // Force a selection
    if (!currentIndex().isValid())
        setCurrentIndex(model()->index(0, 0));
}
