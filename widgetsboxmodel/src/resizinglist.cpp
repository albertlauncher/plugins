// Copyright (c) 2022-2024 Manuel Schneider

#include "resizinglist.h"
#include "itemdelegatebase.h"
#include <QKeyEvent>

ResizingList::ResizingList(QWidget *parent) : QListView(parent)
{
    connect(this, &ResizingList::clicked, this, &ResizingList::activated, Qt::QueuedConnection);

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
    return {width(), contentsMargins().bottom() + contentsMargins().top() + sizeHintForRow(0) * std::min(static_cast<int>(maxItems_), model()->rowCount(rootIndex()))};
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
