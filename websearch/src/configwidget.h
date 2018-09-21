// Copyright (C) 2014-2018 Manuel Schneider

#pragma once
#include <QWidget>
#include "ui_configwidget.h"

namespace Websearch {

class Extension;
class EnginesModel;

class ConfigWidget final : public QWidget
{
    Q_OBJECT

public:

    explicit ConfigWidget(Extension *extension, QWidget *parent = 0);
    ~ConfigWidget();
    Ui::ConfigWidget ui;

private:

    void onActivated(QModelIndex index);
    void onButton_new();
    void onButton_remove();
    void onButton_restoreDefaults();

    Extension *extension_;
    EnginesModel *enginesModel_;
};

}
