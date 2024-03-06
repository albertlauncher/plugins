// Copyright (c) 2023-2024 Manuel Schneider

#pragma once
#include "albert/plugin/plugininstance.h"
#include <QObject>
#include <memory>

class Plugin : public QObject, public albert::PluginInstance
{
    Q_OBJECT ALBERT_PLUGIN

public:
    Plugin();
    ~Plugin();
    QWidget *buildConfigWidget() override;

protected:
    class Private;
    std::unique_ptr<Private> d;
};
