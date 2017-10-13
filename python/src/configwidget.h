// Copyright (c) 2017 Manuel Schneider

#pragma once
#include <QWidget>
#include "ui_configwidget.h"

namespace Python {

class ConfigWidget final : public QWidget
{
    Q_OBJECT
public:
    explicit ConfigWidget(QWidget *parent = nullptr);
    ~ConfigWidget();
    Ui::ConfigWidget ui;
};
}
