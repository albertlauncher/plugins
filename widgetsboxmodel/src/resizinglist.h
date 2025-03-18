// Copyright (c) 2022-2025 Manuel Schneider

#pragma once
#include <QListView>
#include <QStyledItemDelegate>


class ItemDelegateBase : public QStyledItemDelegate
{
public:
    ItemDelegateBase();

    QFont text_font;
    QColor text_color;
    QFontMetrics text_font_metrics;
    QColor selection_text_color;
    QBrush selection_background_brush;
    QBrush selection_border_brush;
    double selection_border_radius;
    double selection_border_width;
    int padding;
    bool draw_debug_overlays;

protected:

    void paint(QPainter *painter, const QStyleOptionViewItem &options, const QModelIndex &index) const override;

};


class ResizingList : public QListView
{
public:

    ResizingList(QWidget *parent = nullptr);

    bool debugMode() const;
    void setDebugMode(bool);

    uint textFontSize() const;
    void setTextFontSize(uint);

    QColor textColor() const;
    void setTextColor(QColor);

    QColor selectionTextColor() const;
    void setSelectionTextColor(QColor);

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
