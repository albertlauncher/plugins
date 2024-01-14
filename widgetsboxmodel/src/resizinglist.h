// Copyright (c) 2022-2024 Manuel Schneider

#pragma once
#include <QListView>


class ResizingList : public QListView
{
public:

    ResizingList(QWidget *parent = nullptr);

    uint maxItems() const;
    void setMaxItems(uint maxItems);

    QSize minimumSizeHint() const override;
    QSize sizeHint() const override;
    void setModel(QAbstractItemModel*) override;

private:

    bool eventFilter(QObject*, QEvent *event) override;
    uint maxItems_;

};
