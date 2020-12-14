// Copyright (C) 2014-2018 Manuel Schneider

#pragma once
#include <QWidget>
#include "ui_configwidget.h"

namespace ExternalExtensions {

class ConfigWidget final : public QWidget
{
    Q_OBJECT
public:
    explicit ConfigWidget(QWidget *parent = 0);
    ~ConfigWidget();
    Ui::ConfigWidget ui;
};

}
