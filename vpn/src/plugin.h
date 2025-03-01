// Copyright (c) 2023-2025 Manuel Schneider

#pragma once

#include <albert/extensionplugin.h>
#include <albert/item.h>
#include <albert/indexqueryhandler.h>
#include <vector>
#include <memory>



class Plugin : public albert::ExtensionPlugin,
               public albert::IndexQueryHandler
{
    ALBERT_PLUGIN
public:

    Plugin();
    ~Plugin();
    void updateIndexItems() override;

private:

    class Private;
    std::unique_ptr<Private> d;
    
};
