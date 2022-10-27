// Copyright (c) 2022 Manuel Schneider

#pragma once
#include <QListView>


class ResizingList : public QListView
{
    Q_OBJECT
    Q_PROPERTY(int maxItems READ maxItems WRITE setMaxItems MEMBER maxItems_ NOTIFY maxItemsChanged)

public:
    explicit ResizingList(QWidget *parent = nullptr);

    uint maxItems() const;
    void setMaxItems(uint maxItems);

    QSize minimumSizeHint() const override;
    QSize sizeHint() const override;
    void setModel(QAbstractItemModel * model) override;

protected:
    bool eventFilter(QObject*, QEvent *event) override;

private:
    uint maxItems_;

signals:
    void maxItemsChanged();

};
