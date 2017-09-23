// albert - a simple application launcher for linux
// Copyright (C) 2014-2015 Manuel Schneider
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#include "configwidget.h"

/** ***************************************************************************/
KeyValueStore::ConfigWidget::ConfigWidget(QSqlDatabase *db, QWidget *parent) : QWidget(parent) {
    ui.setupUi(this);

    model = new QSqlTableModel(this, *db);
    model->setTable("kv");
    model->setEditStrategy(QSqlTableModel::OnFieldChange);
    model->setHeaderData(0, Qt::Horizontal, tr("Key"));
    model->setHeaderData(1, Qt::Horizontal, tr("Value"));
    model->setSort(0, Qt::SortOrder::AscendingOrder);
    model->select();

    ui.tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    ui.tableView->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    ui.tableView->setModel(model);

}


/** ***************************************************************************/
KeyValueStore::ConfigWidget::~ConfigWidget() {

}


/** ***************************************************************************/
void KeyValueStore::ConfigWidget::updateTable() {
    model->select();
}
