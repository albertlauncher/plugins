// Copyright (C) 2014-2021 Manuel Schneider

#pragma once
#include <QAbstractTableModel>

class Plugin;

class EnginesModel final : public QAbstractTableModel
{
    Q_OBJECT

public:

    EnginesModel(Plugin *extension, QObject *parent = Q_NULLPTR);

    int rowCount(const QModelIndex & parent = QModelIndex()) const override;
    int columnCount(const QModelIndex & parent = QModelIndex()) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    QVariant data(const QModelIndex & index, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex & index, const QVariant & value, int role = Qt::EditRole) override;
    Qt::ItemFlags flags(const QModelIndex & index) const override;
    bool insertRows (int position, int rows, const QModelIndex & parent = QModelIndex()) override;
    bool removeRows (int position, int rows, const QModelIndex & parent = QModelIndex()) override;
    bool moveRows(const QModelIndex &sourceRow, int srcRow, int cnt, const QModelIndex & dst, int destinationChild) override;
    Qt::DropActions supportedDropActions() const override;
    bool dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent) override;

    void restoreDefaults();

private:

    Plugin *extension_;

};
