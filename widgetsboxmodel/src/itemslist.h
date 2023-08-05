// Copyright (c) 2022 Manuel Schneider

#pragma once
#include <QEvent>
#include "resizinglist.h"
#include <QStyledItemDelegate>


class ItemDelegate final : public QStyledItemDelegate
{
public:
    ItemDelegate(QObject *parent = nullptr) : QStyledItemDelegate(parent) {}

    void paint(QPainter *painter, const QStyleOptionViewItem &options, const QModelIndex &index) const override;
//    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &) const override {
//        QFont subfont = option.font;
//        subfont.setPixelSize(subtext_size);
//        return QSize(option.rect.width(),
//                     std::max({/*option.rect.height(),*/
//                               drawIcon ? option.rect.height() : 0,
//                               QFontMetrics(option.font).height() + QFontMetrics(subfont).height()}));
//    }

    bool debug = true;
    int text_size;
    uint subtext_size;
};

class ItemsList final : public ResizingList
{
    Q_OBJECT
public:
    explicit ItemsList(QWidget *parent = nullptr);

    Q_PROPERTY(uint description_font_size READ subTextSize WRITE setSubTextSize DESIGNABLE true)

    void setSubTextSize(uint v) {
        qCritical() << "SET PROPERTY TO" << v;
        delegate_->subtext_size = v;
    }
    uint subTextSize() { return delegate_->subtext_size; }

private:
    ItemDelegate *delegate_;
};
