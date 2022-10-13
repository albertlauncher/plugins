// Copyright (C) 2014-2020 Manuel Schneider

#include "albert.h"
#include "ui_configwidget.h"
#include "plugin.h"
using namespace std;


Plugin::Plugin()
        : Core::Extension("org.albert.extension.projectid"), // Must match the id in metadata
          Core::QueryHandler(Core::Plugin::id())
{
    // You can 'throw' in the constructor if something fatal happened.
    // E.g. `throw "Description of error.";`
}

//
//QWidget *Plugin::createSettingsWidget() const
//{
//    auto *widget = new QWidget;
//    widget->setObjectName(metadata.name);
//
//    Ui::ConfigWidget ui;
//    ui.setupUi(widget);
//    return widget;
//}

void Plugin::handleQuery(Core::Query *) const
{
    // Check the include headers to see how things work.
    // In virtually any case you want to add albert::Item's or one of its subclasses
}
