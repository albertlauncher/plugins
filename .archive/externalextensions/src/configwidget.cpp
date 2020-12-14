// Copyright (C) 2014-2018 Manuel Schneider

#include "configwidget.h"

/** ***************************************************************************/
ExternalExtensions::ConfigWidget::ConfigWidget(QWidget *parent) : QWidget(parent) {
    ui.setupUi(this);
    ui.tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    ui.tableView->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
}



/** ***************************************************************************/
ExternalExtensions::ConfigWidget::~ConfigWidget() {

}
