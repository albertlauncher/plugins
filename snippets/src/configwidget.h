// Copyright (C) 2014-2018 Manuel Schneider

#pragma once
#include <QWidget>
#include <QSqlTableModel>
#include <QSqlDatabase>
#include <QSortFilterProxyModel>
#include "ui_configwidget.h"

namespace Snippets {
class ConfigWidget final : public QWidget
{
    Q_OBJECT

public:

    explicit ConfigWidget(QSqlDatabase *db, QWidget *parent = nullptr);
    ~ConfigWidget();
    Ui::ConfigWidget ui;

private:

    void onItemActivated(QModelIndex);
    void onAddClicked();
    void onRemoveClicked();

    QSqlTableModel *model;
    QSortFilterProxyModel *proxyModel;

};
}
