// Copyright (c) 2022-2024 Manuel Schneider

#pragma once
#include "pluginbase.h"

class Plugin : public PluginBase
{
    ALBERT_PLUGIN

public:

    Plugin();

    // albert::ExtensionPlugin
    QWidget *buildConfigWidget() override;

};
