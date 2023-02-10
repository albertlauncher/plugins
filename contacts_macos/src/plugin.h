// Copyright (c) 2022 Manuel Schneider

#pragma once
#include "albert.h"

class Plugin :
        public albert::ExtensionPlugin,
        public albert::GlobalQueryHandler
{
    Q_OBJECT ALBERT_PLUGIN
public:
    QString defaultTrigger() const override { return QStringLiteral("cn "); }
    std::vector<albert::RankItem> handleQuery(const Query&) const override;
};
