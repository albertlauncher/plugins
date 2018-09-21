// Copyright (C) 2014-2018 Manuel Schneider

#include "frontendwidget.h"
#include "frontendplugin.h"


/** ***************************************************************************/
WidgetBoxModel::FrontendPlugin::FrontendPlugin()
    : Frontend("org.albert.frontend.widgetboxmodel"),
      frontendWidget_(new FrontendWidget(&settings())){

    connect(frontendWidget_.get(), &FrontendWidget::inputChanged,
            this, &Frontend::inputChanged);

    connect(frontendWidget_.get(), &FrontendWidget::settingsWidgetRequested,
            this, &Frontend::settingsWidgetRequested);

    connect(frontendWidget_.get(), &FrontendWidget::widgetShown,
            this, &Frontend::widgetShown);

    connect(frontendWidget_.get(), &FrontendWidget::widgetHidden,
            this, &Frontend::widgetHidden);
}


/** ***************************************************************************/
WidgetBoxModel::FrontendPlugin::~FrontendPlugin() {
}


/** ***************************************************************************/
bool WidgetBoxModel::FrontendPlugin::isVisible() {
    return frontendWidget_->isVisible();
}


/** ***************************************************************************/
void WidgetBoxModel::FrontendPlugin::setVisible(bool visible) {
    frontendWidget_->setVisible(visible);
}


/** ***************************************************************************/
QString WidgetBoxModel::FrontendPlugin::input() {
    return frontendWidget_->input();
}


/** ***************************************************************************/
void WidgetBoxModel::FrontendPlugin::setInput(const QString &input) {
    frontendWidget_->setInput(input);
}


/** ***************************************************************************/
void WidgetBoxModel::FrontendPlugin::setModel(QAbstractItemModel *m) {
    frontendWidget_->setModel(m);
}


/** ***************************************************************************/
QWidget *WidgetBoxModel::FrontendPlugin::widget(QWidget *parent) {
    return frontendWidget_->widget(parent);
}

