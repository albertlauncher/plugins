// Copyright (c) 2023 Manuel Schneider

#pragma once
#include "albert.h"

class Plugin final : public albert::ExtensionPlugin,
                     public albert::GlobalQueryHandler
{
    Q_OBJECT ALBERT_PLUGIN
public:
    std::vector<albert::RankItem> handleQuery(const Query&) const override;
};
