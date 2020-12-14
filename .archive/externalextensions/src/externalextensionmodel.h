// Copyright (C) 2014-2018 Manuel Schneider

#pragma once
#include <QAbstractTableModel>
#include <memory>
#include <vector>
#include "externalextension.h"

namespace ExternalExtensions {

class ExternalExtensionsModel : public QAbstractTableModel
{
    Q_OBJECT

public:

    ExternalExtensionsModel(const std::vector<std::unique_ptr<ExternalExtension>> &exts, QObject *parent = Q_NULLPTR)
        : QAbstractTableModel(parent), externalExtensions_(exts) {}

    int rowCount(const QModelIndex & parent = QModelIndex()) const override;
    int columnCount(const QModelIndex & parent = QModelIndex()) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    QVariant data(const QModelIndex & index, int role = Qt::DisplayRole) const override;
    Qt::ItemFlags flags(const QModelIndex & index) const override;

    void onActivated(const QModelIndex &);

private:

    const std::vector<std::unique_ptr<ExternalExtension>> &externalExtensions_;
};

}
