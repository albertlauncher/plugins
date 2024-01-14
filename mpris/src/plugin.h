// Copyright (c) 2017-2024 Manuel Schneider

#pragma once
#include "albert/extension/queryhandler/globalqueryhandler.h"
#include "albert/plugin.h"

class Plugin : public albert::plugin::ExtensionPlugin,
               public albert::GlobalQueryHandler
{
    Q_OBJECT ALBERT_PLUGIN
public:
    Plugin();
    ~Plugin();

    std::vector<albert::RankItem> handleGlobalQuery(const GlobalQuery *) const override;
    QWidget *buildConfigWidget() override;

private:
    struct Private;
    std::unique_ptr<Private> d;
};
