// Copyright (c) 2022 Manuel Schneider

#pragma once
#include "albert.h"

class Plugin:
        public albert::ExtensionPlugin,
        public albert::GlobalQueryHandler
{
    Q_OBJECT ALBERT_PLUGIN
public:
    Plugin();
    std::vector<albert::RankItem> handleGlobalQuery(const GlobalQuery&) const override;
private:
    QStringList valid_tlds;
};
