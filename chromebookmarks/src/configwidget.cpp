// Copyright (C) 2014-2018 Manuel Schneider

#include <QFileDialog>
#include <QTimer>
#include "configwidget.h"

/** ***************************************************************************/
ChromeBookmarks::ConfigWidget::ConfigWidget(QWidget *parent) : QWidget(parent) {
    ui.setupUi(this);

    connect(ui.pushButton_editPath, &QPushButton::clicked,
            this, &ConfigWidget::onButton_EditPath);
}

/** ***************************************************************************/
ChromeBookmarks::ConfigWidget::~ConfigWidget() {

}

/** ***************************************************************************/
void ChromeBookmarks::ConfigWidget::onButton_EditPath() {
    QString path = QFileDialog::getOpenFileName(this, tr("Choose path"));
    if(path.isEmpty()) return;
    emit requestEditPath(path);
}
