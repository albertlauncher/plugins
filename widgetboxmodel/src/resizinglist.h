// Copyright (C) 2014-2018 Manuel Schneider

#pragma once
#include <QListView>

namespace WidgetBoxModel {

class ResizingList : public QListView
{
    Q_OBJECT
    Q_PROPERTY(int maxItems READ maxItems WRITE setMaxItems MEMBER maxItems_ NOTIFY maxItemsChanged)

public:

    ResizingList(QWidget *parent = 0) : QListView(parent), maxItems_(5) {}
    virtual ~ResizingList() {}

    uint maxItems() const;
    void setMaxItems(uint maxItems);

    QSize minimumSizeHint() const override;
    QSize sizeHint() const override;
    void setModel(QAbstractItemModel * model) override;

private:

    void updateAppearance();

    uint maxItems_;

signals:

    void maxItemsChanged();

};

}
