// Copyright (c) 2022 Manuel Schneider

#pragma once
#include "albert/extension/queryhandler/globalqueryhandler.h"
#include "albert/plugin.h"

class Plugin : public albert::plugin::ExtensionPlugin,
               public albert::GlobalQueryHandler
{
    Q_OBJECT ALBERT_PLUGIN

public:

    QWidget *buildConfigWidget() override;
    QString defaultTrigger() const override { return QStringLiteral("cn "); }
    std::vector<albert::RankItem> handleGlobalQuery(const GlobalQuery*) const override;

};
