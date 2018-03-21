// Copyright (C) 2014-2018 Manuel Schneider

#include <QDebug>
#include "configwidget.h"
#include "snippeteditordialog.h"

/** ***************************************************************************/
Snippets::ConfigWidget::ConfigWidget(QSqlDatabase *db, QWidget *parent) : QWidget(parent) {
    ui.setupUi(this);

    model = new QSqlTableModel(this, *db);
    model->setTable("snippets");
    model->setHeaderData(0, Qt::Horizontal, tr("Title"));
    model->setHeaderData(1, Qt::Horizontal, tr("Text"));
    model->setSort(0, Qt::SortOrder::AscendingOrder);
    model->select();

    ui.tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    ui.tableView->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    ui.tableView->setModel(model);

    connect(ui.tableView, &QTableView::activated,
            this, &ConfigWidget::onItemActivated);

    connect(ui.toolButton_add, &QToolButton::clicked,
            this, &ConfigWidget::onAddClicked);

    connect(ui.toolButton_remove, &QToolButton::clicked,
            this, &ConfigWidget::onRemoveClicked);
}

/** ***************************************************************************/
Snippets::ConfigWidget::~ConfigWidget() {

}

/** ***************************************************************************/
void Snippets::ConfigWidget::updateTable() {
    model->select();
}

/** ***************************************************************************/
void Snippets::ConfigWidget::onItemActivated(QModelIndex index) {

    QModelIndex keywordIdx = model->index(index.row(), 0);
    QModelIndex snippetIdx = model->index(index.row(), 1);
    SnippetEditorDialog editor;

    editor.ui.lineEdit->setText(keywordIdx.data().toString());
    editor.ui.plainTextEdit->setPlainText(snippetIdx.data().toString());

    if (editor.exec()) {
        model->setData(keywordIdx, editor.ui.lineEdit->text());
        model->setData(snippetIdx, editor.ui.plainTextEdit->toPlainText());
    }
}

/** ***************************************************************************/
void Snippets::ConfigWidget::onAddClicked() {
    SnippetEditorDialog editor;
    if (editor.exec()) {
        int row = model->rowCount();
        model->insertRow(row);
        model->setData(model->index(row, 0), editor.ui.lineEdit->text());
        model->setData(model->index(row, 1), editor.ui.plainTextEdit->toPlainText());
        model->submitAll();
    }
}

/** ***************************************************************************/
void Snippets::ConfigWidget::onRemoveClicked() {
    QModelIndex idx = ui.tableView->currentIndex();
    if (idx.isValid())
        model->removeRows(idx.row(), 1);
    model->select();
    ui.tableView->reset();
}
