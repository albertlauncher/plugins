// Copyright (C) 2014-2018 Manuel Schneider

#include "frontendplugin.h"
#include "configwidget.h"
#include "mainwindow.h"


/** ***************************************************************************/
QmlBoxModel::FrontendPlugin::FrontendPlugin()
    : Frontend("org.albert.frontend.qmlboxmodel"),
      mainWindow(new MainWindow(this)){

    connect(mainWindow.get(), &MainWindow::inputChanged, [this](){
        emit inputChanged(input());
    });

    connect(mainWindow.get(), &MainWindow::settingsWidgetRequested,
            this, &Frontend::settingsWidgetRequested);

    connect(mainWindow.get(), &MainWindow::visibilityChanged,
            this, [this](QWindow::Visibility visibility){
        emit ( visibility == QWindow::Visibility::Hidden ) ? widgetHidden() : widgetShown();
    });
}


/** ***************************************************************************/
QmlBoxModel::FrontendPlugin::~FrontendPlugin() {

}


/** ***************************************************************************/
bool QmlBoxModel::FrontendPlugin::isVisible() {
    return mainWindow->isVisible();
}


/** ***************************************************************************/
void QmlBoxModel::FrontendPlugin::setVisible(bool visible) {
    mainWindow->setVisible(visible);
    mainWindow->raise();
    mainWindow->requestActivate();
}


/** ***************************************************************************/
QString QmlBoxModel::FrontendPlugin::input() {
    return mainWindow->input();
}


/** ***************************************************************************/
void QmlBoxModel::FrontendPlugin::setInput(const QString &input) {
    mainWindow->setInput(input);
}


/** ***************************************************************************/
void QmlBoxModel::FrontendPlugin::setModel(QAbstractItemModel *m) {
    mainWindow->setModel(m);
}


/** ***************************************************************************/
QWidget *QmlBoxModel::FrontendPlugin::widget(QWidget *parent) {
    return new ConfigWidget(mainWindow.get(), parent);
}

