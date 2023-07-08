// Copyright (c) 2023 Manuel Schneider

#pragma once
#include "albert/extension/queryhandler/globalqueryhandler.h"
#include "albert/plugin.h"


class TimeZoneHandler : public albert::TriggerQueryHandler
{
public:
    QString id() const override;
    QString name() const override;
    QString description() const override;
    QString synopsis() const override;
    QString defaultTrigger() const override;
    void handleTriggerQuery(TriggerQuery*) const override;
};


class Plugin : public albert::plugin::ExtensionPlugin<albert::GlobalQueryHandler>
{
    Q_OBJECT ALBERT_PLUGIN
public:
    std::vector<albert::RankItem> handleGlobalQuery(const GlobalQuery*) const override;
    std::vector<albert::Extension*> extensions() override;

private:
    TimeZoneHandler tzh;
};
