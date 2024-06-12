// Copyright (c) 2022-2024 Manuel Schneider

#include "primitives.h"
#include "resizinglist.h"
#include "style.h"
#include <QKeyEvent>
#include <QPixmapCache>
#include <albert/logging.h>

ResizingList::ResizingList(QWidget *parent) : QListView(parent), maxItems_(5)
{
    setEditTriggers(QAbstractItemView::NoEditTriggers);
    setFrameShape(QFrame::NoFrame);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    viewport()->setAutoFillBackground(false);
    connect(this, &ResizingList::clicked, this,
            &ResizingList::activated, Qt::QueuedConnection);
}

uint ResizingList::maxItems() const
{ return maxItems_; }

void ResizingList::setMaxItems(uint maxItems)
{
    maxItems_ = maxItems;
    updateGeometry();
}

QSize ResizingList::sizeHint() const
{
    if (model() == nullptr)
        return {};
    auto count = std::min(static_cast<int>(maxItems_), model()->rowCount(rootIndex()));
    return {width(), contentsMargins().bottom()
                     + contentsMargins().top()
                     + count * (sizeHintForRow(0) + 2 * spacing())};
}

QSize ResizingList::minimumSizeHint() const { return {0,0}; }

void ResizingList::setModel(QAbstractItemModel *m)
{
    if (model() != nullptr)
        disconnect(model(), nullptr, this, nullptr);

    if (m != nullptr)
    {
        connect(m, &QAbstractItemModel::rowsInserted, this, &ResizingList::updateGeometry);
        connect(m, &QAbstractItemModel::modelReset, this, &ResizingList::updateGeometry);
    }

    QAbstractItemView::setModel(m);
    updateGeometry();
}

void ResizingList::setStyle(const Style *s)
{
    style = s;

    // auto p = s->item_view_padding + s->item_view_border_width;
   //setViewportMargins(p, p, p, p);
    // setContentsMargins(p, p, p, p);

    update();
}

void ResizingList::paintEvent(QPaintEvent *event)
{
    // Draw Frame if necessary
    if (!(style->item_view_background_brush == Qt::NoBrush
          && style->item_view_border_brush == Qt::NoBrush))
    {
        QPixmap pm;
        if (const auto cache_key = QStringLiteral("_ItemViewFrame_w%1_h%2")
                                       .arg(width()).arg(height());
            !QPixmapCache::find(cache_key, &pm))
        {
            auto dpr = devicePixelRatioF();
            pm = pixelPerfectRoundedRect(size() * dpr,
                                         style->item_view_background_brush,
                                         style->item_view_border_radius * dpr,
                                         style->item_view_border_brush,
                                         style->item_view_border_width * dpr);
            pm.setDevicePixelRatio(dpr);
            // QPixmapCache::insert(cache_key, pm);
        }

        QPainter p(viewport());
        p.drawPixmap(contentsRect(), pm);
    }

    QListView::paintEvent(event);
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

// bool ResizingList::event(QEvent *event)
// {
//     if (event->type() == QEvent::Show)
//         CRIT << "Show";
//     if (event->type() == QEvent::Hide)
//         CRIT << "Hide";
//     return QListView::event(event);
// }
