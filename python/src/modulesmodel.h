// Copyright (c) 2017-2018 Manuel Schneider

#pragma once
#include <QAbstractTableModel>
#include <memory>
#include <vector>

namespace Python {

class Extension;

class ModulesModel : public QAbstractTableModel
{
    Q_OBJECT

public:

    ModulesModel(Extension *extension, QObject *parent = nullptr);

    int rowCount(const QModelIndex & parent = QModelIndex()) const override;
    int columnCount(const QModelIndex & parent = QModelIndex()) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    QVariant data(const QModelIndex & index, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role) override;
    Qt::ItemFlags flags(const QModelIndex & index) const override;

private:

    Extension *extension;
};

}
