// Copyright (c) 2022 Manuel Schneider

#pragma once
#include "albert.h"

class Plugin :
        public albert::ExtensionPlugin,
        public albert::QueryHandler
{
    Q_OBJECT ALBERT_PLUGIN
public:
    QString defaultTrigger() const override { return QStringLiteral("cn "); }
    std::vector<albert::RankItem> handleGlobalQuery(const GlobalQuery&) const override;
};
