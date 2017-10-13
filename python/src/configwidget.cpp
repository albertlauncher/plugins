// Copyright (c) 2017 Manuel Schneider

#include "configwidget.h"

/** ***************************************************************************/
Python::ConfigWidget::ConfigWidget(QWidget *parent) : QWidget(parent) {
    ui.setupUi(this);
    ui.tableView->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    ui.tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    ui.tableView->horizontalHeader()->setStretchLastSection(true);
    ui.tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
}



/** ***************************************************************************/
Python::ConfigWidget::~ConfigWidget() {

}
