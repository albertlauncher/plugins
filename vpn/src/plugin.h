// Copyright (c) 2023-2024 Manuel Schneider

#pragma once

#include <albert/extensionplugin.h>
#include <albert/indexqueryhandler.h>

class Plugin : public albert::ExtensionPlugin,
               public albert::IndexQueryHandler
{
    ALBERT_PLUGIN
public:
    void updateIndexItems() override;
};
