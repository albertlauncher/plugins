// Copyright (c) 2017-2024 Manuel Schneider

#pragma once
#include "albert/query/indexqueryhandler.h"
#include "albert/util/extensionplugin.h"

class Plugin : public albert::ExtensionPlugin,
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
