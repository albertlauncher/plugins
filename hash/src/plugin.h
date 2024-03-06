// Copyright (c) 2022-2024 Manuel Schneider

#pragma once
#include "albert/query/globalqueryhandler.h"
#include "albert/util/extensionplugin.h"

class Plugin : public albert::ExtensionPlugin,
               public albert::GlobalQueryHandler
{
    Q_OBJECT ALBERT_PLUGIN
public:
    std::vector<albert::RankItem> handleGlobalQuery(const albert::Query*) const override;
    void handleTriggerQuery(albert::Query*) override;
};
