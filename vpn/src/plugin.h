// Copyright (c) 2023-2024 Manuel Schneider

#pragma once
#include "albert/extension/queryhandler/indexqueryhandler.h"
#include "albert/plugin.h"

class Plugin : public albert::plugin::ExtensionPlugin,
               public albert::IndexQueryHandler
{
    Q_OBJECT ALBERT_PLUGIN
public:
    QString synopsis() const override;
    void updateIndexItems() override;

};
