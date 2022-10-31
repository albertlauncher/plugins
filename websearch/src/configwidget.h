// Copyright (C) 2014-2021 Manuel Schneider

#pragma once
#include <QWidget>
#include "ui_configwidget.h"

class Plugin;
class EnginesModel;

class ConfigWidget final : public QWidget
{
    Q_OBJECT

public:

    explicit ConfigWidget(Plugin *extension, QWidget *parent = 0);
    Ui::ConfigWidget ui;

private:

    void onActivated(QModelIndex index);
    void onButton_new();
    void onButton_remove();
    void onButton_restoreDefaults();

    Plugin *plugin_;
    EnginesModel *enginesModel_;
};
