// Copyright (c) 2022-2024 Manuel Schneider

#pragma once
#include "albert/extension/queryhandler/triggerqueryhandler.h"
#include "albert/plugin.h"

class Plugin : public albert::plugin::ExtensionPlugin,
               public albert::TriggerQueryHandler
{
    Q_OBJECT ALBERT_PLUGIN
public:
    QString synopsis() const override;
    QString defaultTrigger() const override;
    void handleTriggerQuery(TriggerQuery*) const override;
};
