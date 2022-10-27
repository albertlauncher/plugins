// Copyright (c) 2022 Manuel Schneider

#pragma once
#include <QEvent>
#include "resizinglist.h"
#include <QStyledItemDelegate>


class ActionsList final : public ResizingList
{
    Q_OBJECT
    class ActionDelegate;
public:
    explicit ActionsList(QWidget *parent = nullptr);
};


class ActionsList::ActionDelegate final : public QStyledItemDelegate
{
public:
    explicit ActionDelegate(QObject *parent = nullptr) : QStyledItemDelegate(parent) {}
    void paint(QPainter *painter, const QStyleOptionViewItem &options, const QModelIndex &index) const override;
};

