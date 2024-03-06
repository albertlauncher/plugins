// Copyright (c) 2022 Manuel Schneider
#pragma once
#include <albert/extensionplugin.h>
#include <albert/triggerqueryhandler.h>

class Plugin : public albert::ExtensionPlugin,
               public albert::TriggerQueryHandler
{
    ALBERT_PLUGIN
public:
    Plugin();
    ~Plugin() override;

    bool allowTriggerRemap() const override;
    QString synopsis() const override;
    void handleTriggerQuery(albert::Query*) override;
};
