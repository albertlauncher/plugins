// Copyright (c) 2023 Manuel Schneider

#pragma once
#include "albert/extension/queryhandler/globalqueryhandler.h"
#include "albert/plugin.h"

class Plugin : public albert::plugin::ExtensionPlugin<albert::GlobalQueryHandler>
{
    Q_OBJECT ALBERT_PLUGIN
public:
    Plugin();
    ~Plugin();

    std::vector<albert::RankItem> handleGlobalQuery(const GlobalQuery *) const override;

private:
    struct Private;
    std::unique_ptr<Private> d;
};
