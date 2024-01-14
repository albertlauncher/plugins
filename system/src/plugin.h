// Copyright (c) 2017-2024 Manuel Schneider

#pragma once
#include "albert/extension/queryhandler/indexqueryhandler.h"
#include "albert/plugin.h"

class Plugin : public albert::plugin::ExtensionPlugin,
               public albert::IndexQueryHandler
{
    Q_OBJECT ALBERT_PLUGIN
public:
    Plugin();

    void updateIndexItems() override;
    QWidget* buildConfigWidget() override;

    std::array<QString, 6> default_title;
    std::array<QString, 6> descriptions;
};
