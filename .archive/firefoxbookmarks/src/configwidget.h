// Copyright (c) 2022 Manuel Schneider

#pragma once
#include <QWidget>
#include "ui_configwidget.h"

namespace FirefoxBookmarks {
class ConfigWidget final : public QWidget
{
    Q_OBJECT
public:
    explicit ConfigWidget(QWidget *parent = 0);
    ~ConfigWidget();
    Ui::ConfigWidget ui;
};
}
