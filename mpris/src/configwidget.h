// Copyright (C) 2016-2017 Martin Buergmann

#pragma once
#include <QWidget>
#include "ui_configwidget.h"

namespace MPRIS {
class ConfigWidget final : public QWidget
{
    Q_OBJECT
public:
    explicit ConfigWidget(QWidget *parent = 0);
    ~ConfigWidget();
    Ui::ConfigWidget ui;
};
}
