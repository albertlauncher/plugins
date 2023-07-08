// Copyright (c) 2022 Manuel Schneider

#pragma once
#include "albert/extension/queryhandler/triggerqueryhandler.h"
#include "albert/plugin.h"


class DictHandler final : public albert::TriggerQueryHandler
{
public:
    QString id() const override;
    QString name() const override;
    QString description() const override;
    QString synopsis() const override;
    QString defaultTrigger() const override;
    void handleTriggerQuery(TriggerQuery*) const override;
};


class Plugin : public albert::plugin::Plugin<>
{
    Q_OBJECT ALBERT_PLUGIN
public:
    std::vector<albert::Extension*> extensions() override;

protected:
    DictHandler dict_handler;
};
