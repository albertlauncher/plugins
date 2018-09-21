// Copyright (C) 2014-2018 Manuel Schneider

#pragma once
#include <QWidget>
#include "ui_configwidget.h"
#include "extension.h"

namespace Debug
{
class ConfigWidget final : public QWidget
{
    Q_OBJECT

public:

    explicit ConfigWidget(Extension * extension_, QWidget * parent = 0);

private:

    Ui::ConfigWidget ui;
    Extension * extension_;

};
}
