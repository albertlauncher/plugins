// Copyright (c) 2023 Manuel Schneider

#pragma once
#include "albert/extension/queryhandler/triggerqueryhandler.h"
#include "albert/plugin.h"

class Plugin : public albert::plugin::ExtensionPlugin<albert::TriggerQueryHandler>
{
    Q_OBJECT ALBERT_PLUGIN

public:
    Plugin();
    void handleTriggerQuery(TriggerQuery*) const override;
    QWidget* buildConfigWidget() override;
};
