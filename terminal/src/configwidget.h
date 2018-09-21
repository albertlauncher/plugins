// Copyright (C) 2014-2018 Manuel Schneider

#pragma once
#include <QWidget>
#include "ui_configwidget.h"

namespace Terminal {

class ConfigWidget final : public QWidget
{
    Q_OBJECT

public:

    ConfigWidget(QWidget *parent) : QWidget(parent) {
        ui.setupUi(this);
    }

    Ui::ConfigWidget ui;

};

}
