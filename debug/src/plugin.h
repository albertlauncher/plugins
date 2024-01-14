// Copyright (c) 2022 Manuel Schneider
#pragma once
#include "albert/plugin.h"
#include "albert/extension/queryhandler/triggerqueryhandler.h"

class Plugin : public albert::plugin::ExtensionPlugin,
               public albert::TriggerQueryHandler
{
    Q_OBJECT ALBERT_PLUGIN
public:
    Plugin();
    ~Plugin() override;

    bool allowTriggerRemap() const override;
    QString synopsis() const override;
    void handleTriggerQuery(TriggerQuery*) const override;
};
