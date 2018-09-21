// Copyright (C) 2014-2018 Manuel Schneider

#include "resizinglist.h"
#include <QDebug>

/** ***************************************************************************/
uint WidgetBoxModel::ResizingList::maxItems() const {
    return maxItems_;
}



/** ***************************************************************************/
void WidgetBoxModel::ResizingList::setMaxItems(uint maxItems) {
    maxItems_ = maxItems;
    updateGeometry();
}



/** ***************************************************************************/
QSize WidgetBoxModel::ResizingList::sizeHint() const {
    if (model() == nullptr)
        return QSize();
    return QSize(width(), sizeHintForRow(0) * std::min(static_cast<int>(maxItems_), model()->rowCount(rootIndex())));
}



/** ***************************************************************************/
QSize WidgetBoxModel::ResizingList::minimumSizeHint() const {
    return QSize(0,0); // Fix for small lists
}



/** ***************************************************************************/
void WidgetBoxModel::ResizingList::setModel(QAbstractItemModel * m) {
    if (model() == m)
        return;

    if (model()!=nullptr) {
        disconnect(this->model(), &QAbstractItemModel::rowsInserted, this, &ResizingList::updateAppearance);
        disconnect(this->model(), &QAbstractItemModel::modelReset, this, &ResizingList::updateAppearance);
    }

    QItemSelectionModel *sm = selectionModel();
    QAbstractItemView::setModel(m);
    delete sm;
    updateAppearance();

    // If not empty show and select first, update geom. If not null connect.
    if (model()!=nullptr) {
        connect(this->model(), &QAbstractItemModel::rowsInserted, this, &ResizingList::updateAppearance);
        connect(this->model(), &QAbstractItemModel::modelReset, this, &ResizingList::updateAppearance);
    }
}



/** ***************************************************************************/
void WidgetBoxModel::ResizingList::updateAppearance() {
    if ( model() == nullptr || model()->rowCount() == 0 )
        hide();
    else {
        show();
        if ( !currentIndex().isValid() )
            setCurrentIndex(model()->index(0, 0));
        updateGeometry();
    }
}

