// Copyright (c) 2022 Manuel Schneider

#pragma once
#include "ui_configwidget.h"
#include <QStringListModel>
#include <QTimer>
#include <QWidget>

class Plugin;

class ConfigWidget final : public QWidget
{
    Q_OBJECT
public:
    explicit ConfigWidget(Plugin *, QWidget *parent = 0);
    Ui::ConfigWidget ui;
private:
    void adjustMimeCheckboxes();
    QStringListModel paths_model;
    QString current_path;
    Plugin *plugin;
};
