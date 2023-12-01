// Copyright (c) 2022 Manuel Schneider

#pragma once
#include "albert/extension/queryhandler/indexqueryhandler.h"
#include "albert/plugin.h"

class Plugin : public albert::plugin::ExtensionPlugin<albert::IndexQueryHandler>
{
    Q_OBJECT ALBERT_PLUGIN
public:
    QString synopsis() const override;
    void updateIndexItems() override;

};
