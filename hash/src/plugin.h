// Copyright (c) 2022-2024 Manuel Schneider

#pragma once
#include <albert/globalqueryhandler.h>
#include <albert/extensionplugin.h>

class Plugin : public albert::ExtensionPlugin,
               public albert::GlobalQueryHandler
{
    ALBERT_PLUGIN
public:
    std::vector<albert::RankItem> handleGlobalQuery(const albert::Query*) const override;
    void handleTriggerQuery(albert::Query*) override;
};
