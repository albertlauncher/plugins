// Copyright (c) 2022-2024 Manuel Schneider

#include "resizinglist.h"
#include <QKeyEvent>

ResizingList::ResizingList(QWidget *parent) : QListView(parent), maxItems_(5)
{
    setFrameShape(QFrame::NoFrame);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
//    setLayoutMode(LayoutMode::Batched);
    setUniformItemSizes(true);
    connect(this, &ResizingList::clicked, this, &ResizingList::activated, Qt::QueuedConnection);
    hide();
}

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

// bool ResizingList::event(QEvent *event)
// {
//     if (event->type() == QEvent::Show)
//         CRIT << "Show";
//     if (event->type() == QEvent::Hide)
//         CRIT << "Hide";
//     return QListView::event(event);
// }
