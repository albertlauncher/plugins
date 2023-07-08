// Copyright (c) 2022 Manuel Schneider

#pragma once
#include "albert/extension/queryhandler/globalqueryhandler.h"
#include "albert/plugin.h"

class Plugin : public albert::plugin::ExtensionPlugin<albert::GlobalQueryHandler>
{
    Q_OBJECT ALBERT_PLUGIN
public:
    std::vector<albert::RankItem> handleGlobalQuery(const GlobalQuery*) const override;
    void handleTriggerQuery(TriggerQuery*) const override;
};
