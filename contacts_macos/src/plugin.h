// Copyright (c) 2022 Manuel Schneider

#pragma once
#include <albert/globalqueryhandler.h>
#include <albert/extensionplugin.h>

class Plugin : public albert::ExtensionPlugin,
               public albert::GlobalQueryHandler
{
    ALBERT_PLUGIN

public:

    QWidget *buildConfigWidget() override;
    QString defaultTrigger() const override { return QStringLiteral("cn "); }
    std::vector<albert::RankItem> handleGlobalQuery(const albert::Query*) override;

};
