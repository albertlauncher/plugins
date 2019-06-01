// Copyright (C) 2014-2018 Manuel Schneider

#include <QFileDialog>
#include <QTimer>
#include "configwidget.h"

/** ***************************************************************************/
BraveBookmarks::ConfigWidget::ConfigWidget(QWidget *parent) : QWidget(parent) {
    ui.setupUi(this);

    connect(ui.pushButton_editPath, &QPushButton::clicked,
            this, &ConfigWidget::onButton_EditPath);
}

/** ***************************************************************************/
BraveBookmarks::ConfigWidget::~ConfigWidget() {

}

/** ***************************************************************************/
void BraveBookmarks::ConfigWidget::onButton_EditPath() {
    QString path = QFileDialog::getOpenFileName(this, tr("Choose path"));
    if(path.isEmpty()) return;
    emit requestEditPath(path);
}
