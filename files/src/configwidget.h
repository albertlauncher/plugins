// Copyright (C) 2014-2018 Manuel Schneider

#pragma once
#include <QWidget>
#include <QTimer>
#include "ui_configwidget.h"

namespace Files {

class Extension;

class ConfigWidget final : public QWidget
{
    Q_OBJECT

public:

    explicit ConfigWidget(Extension *ext, QWidget *parent = 0);
    ~ConfigWidget();

private:

    Extension *extension;
    Ui::ConfigWidget ui;

signals:

    void requestAddPath(const QString&);
    void requestRemovePath(const QString&);

};

}
