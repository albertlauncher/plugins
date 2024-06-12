// Copyright (c) 2022-2024 Manuel Schneider

#pragma once
#include <QListView>
class Style;

class ResizingList : public QListView
{
public:

    ResizingList(QWidget *parent);

    uint maxItems() const;
    void setMaxItems(uint maxItems);

    void setModel(QAbstractItemModel*) override;

    void setStyle(const Style*);

protected:

    void paintEvent(QPaintEvent *event) override;
    bool eventFilter(QObject*, QEvent *event) override;
    QSize minimumSizeHint() const override;
    QSize sizeHint() const override;
    const Style *style;

    uint maxItems_;

};
