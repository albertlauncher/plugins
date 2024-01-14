// Copyright (c) 2023 Manuel Schneider

#pragma once
#include "albert/plugin.h"
#include <memory>

class Plugin : public albert::plugin::Plugin
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
