// Copyright (c) 2022-2024 Manuel Schneider

#pragma once
#include <QListView>
class ItemDelegateBase;

class ResizingList : public QListView
{
public:

    ResizingList(QWidget *parent = nullptr);

    QBrush selectionBackgroundBrush() const;
    void setSelectionBackgroundBrush(QBrush);

    QBrush selectionBorderBrush() const;
    void setSelectionBorderBrush(QBrush);

    double borderRadius() const;
    void setBorderRadius(double);

    double borderWidth() const;
    void setBorderWidth(double);

    uint padding() const;
    void setPadding(uint);

    uint maxItems() const;
    void setMaxItems(uint maxItems);

    void setModel(QAbstractItemModel*) override;

    QSize minimumSizeHint() const override;
    QSize sizeHint() const override;

protected:

    bool eventFilter(QObject*, QEvent *event) override;

    void relayout();

private:

    virtual ItemDelegateBase *delegate() const = 0;

    uint maxItems_;

};
