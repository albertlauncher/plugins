// Copyright (c) 2023 Manuel Schneider

#include <QFileDialog>
#include <QMessageBox>
#include <QStandardPaths>
#include "configwidget.h"
#include "enginesmodel.h"
#include "plugin.h"
#include "searchengineeditor.h"


ConfigWidget::ConfigWidget(Plugin *plugin, QWidget *parent)
    : QWidget(parent), plugin_(plugin)
{
    ui.setupUi(this);

    enginesModel_ = new EnginesModel(plugin, ui.tableView_searches);
    ui.tableView_searches->setModel(enginesModel_);

    ui.tableView_searches->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    ui.tableView_searches->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);

    // Initialize connections
    connect(ui.pushButton_new, &QPushButton::clicked,
            this, &ConfigWidget::onButton_new);

    connect(ui.pushButton_remove, &QPushButton::clicked,
            this, &ConfigWidget::onButton_remove);

    connect(ui.pushButton_restoreDefaults, &QPushButton::clicked,
            this, &ConfigWidget::onButton_restoreDefaults);

    connect(ui.tableView_searches, &QTableView::activated,
            this, &ConfigWidget::onActivated);
}


void ConfigWidget::onActivated(QModelIndex index)
{
    int row = index.row();
    SearchEngineEditor searchEngineEditor(plugin_->engines()[static_cast<ulong>(row)], this);

    if (searchEngineEditor.exec()){
        // Set the new engine
        const SearchEngine & searchEngine = searchEngineEditor.searchEngine();
        enginesModel_->setData(enginesModel_->index(row, 0), searchEngine.name, Qt::DisplayRole);
        enginesModel_->setData(enginesModel_->index(row, 0), searchEngine.iconPath, Qt::DecorationRole);
        enginesModel_->setData(enginesModel_->index(row, 1), searchEngine.trigger, Qt::DisplayRole);
        enginesModel_->setData(enginesModel_->index(row, 2), searchEngine.url, Qt::DisplayRole);
    }
    ui.tableView_searches->reset();
}


void ConfigWidget::onButton_new()
{
    // Open search engine editor
    SearchEngine searchEngine;
    searchEngine.iconPath = ":default";
    SearchEngineEditor searchEngineEditor(searchEngine, this);

    if (searchEngineEditor.exec()){

        // Insert new row in model
        int row = (ui.tableView_searches->currentIndex().isValid())
                ? ui.tableView_searches->currentIndex().row()
                : ui.tableView_searches->model()->rowCount();
        enginesModel_->insertRow(row);

        // Set the new engine
        searchEngine = searchEngineEditor.searchEngine();
        enginesModel_->setData(enginesModel_->index(row, 0), searchEngine.name, Qt::DisplayRole);
        enginesModel_->setData(enginesModel_->index(row, 0), searchEngine.iconPath, Qt::DecorationRole);
        enginesModel_->setData(enginesModel_->index(row, 1), searchEngine.trigger, Qt::DisplayRole);
        enginesModel_->setData(enginesModel_->index(row, 2), searchEngine.url, Qt::DisplayRole);

        // Set current
        QModelIndex index = ui.tableView_searches->model()->index(row, 0, QModelIndex());
        ui.tableView_searches->setCurrentIndex(index);
    }
}


void ConfigWidget::onButton_remove()
{
    auto index = ui.tableView_searches->currentIndex();
    if (!index.isValid())
        return;

    // Ask if sure
    QString engineName = ui.tableView_searches->model()
            ->data(ui.tableView_searches->model()->index(index.row(), 1)).toString();
    QMessageBox::StandardButton reply =
            QMessageBox::question(this, "Sure?",
                                  QString("Do you really want to remove '%1' from the search engines?")
                                  .arg(engineName),
                                  QMessageBox::Yes|QMessageBox::No);
    // Remove if sure
    if (reply == QMessageBox::Yes)
        ui.tableView_searches->model()->removeRow(ui.tableView_searches->currentIndex().row());
}


void ConfigWidget::onButton_restoreDefaults()
{
    QMessageBox::StandardButton reply =
            QMessageBox::question(this, "Sure?",
                                  QString("Do you really want to restore the default search engines?"),
                                  QMessageBox::Yes|QMessageBox::No);
    // Remove if sure
    if (reply == QMessageBox::Yes)
        enginesModel_->restoreDefaults();
}
