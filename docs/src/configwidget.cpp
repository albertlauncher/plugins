// Copyright (c) 2022-2024 Manuel Schneider

#include "configwidget.h"
#include "plugin.h"
#include <QAbstractListModel>
#include <QMessageBox>
using namespace albert;
using namespace std;

ConfigWidget::ConfigWidget()
{
    ui.setupUi(this);

    ui.list_view->setModel(&model);

    connect(ui.update_button, &QPushButton::pressed,
            Plugin::instance(), &Plugin::updateDocsetList);

    connect(ui.cancel_button, &QPushButton::pressed,
            Plugin::instance(), &Plugin::cancelDownload);

    connect(Plugin::instance(), &Plugin::statusInfo,
            ui.status_label, &QLabel::setText);

    connect(Plugin::instance(), &Plugin::downloadStateChanged, this, [this]
    {
        ui.list_view->setEnabled(!Plugin::instance()->isDownloading());
        ui.update_button->setEnabled(!Plugin::instance()->isDownloading());
        ui.cancel_button->setVisible(Plugin::instance()->isDownloading());
    });

    ui.cancel_button->hide();
}

DocsetsModel::DocsetsModel()
{
    connect(Plugin::instance(), &Plugin::docsetsChanged,
            this, [this]{ beginResetModel(); endResetModel(); });

    connect(Plugin::instance(), &Plugin::downloadStateChanged,
            this, [this]{ emit dataChanged(index(0), index(rowCount() - 1)); });
}

int DocsetsModel::rowCount(const QModelIndex &) const
{ return Plugin::instance()->docsets().size(); }

QVariant DocsetsModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return {};

    switch (const auto &ds = Plugin::instance()->docsets().at(index.row()); role)
    {
    case Qt::CheckStateRole:
        if (Plugin::instance()->isDownloading())
            return Qt::PartiallyChecked;
        else if (ds.isInstalled())
            return Qt::Checked;
        else
            return Qt::Unchecked;

    case Qt::DecorationRole:
        try {
            return icon_cache.at(ds.icon_path);
        } catch (const out_of_range &e) {
            return icon_cache.emplace(ds.icon_path, ds.icon_path).first->second;
        }

    case Qt::DisplayRole:
        return ds.title;

    case Qt::ToolTipRole:
        return ds.isInstalled() ? QString("%1 %2").arg(ds.name, ds.path) : ds.name;
    }

    return {};
}

bool DocsetsModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (index.isValid() && role == Qt::CheckStateRole && !Plugin::instance()->isDownloading())
    {
        const auto &ds = Plugin::instance()->docsets().at(index.row());

        if (!ds.isInstalled() && value == Qt::Checked)
            Plugin::instance()->downloadDocset(index.row());

        else if (ds.isInstalled() && value == Qt::Unchecked)
            Plugin::instance()->removeDocset(index.row());
        {

            return true;
        }
    }
    return false;
}

Qt::ItemFlags DocsetsModel::flags(const QModelIndex &index) const
{
    return QAbstractListModel::flags(index) | Qt::ItemIsUserCheckable;
}
