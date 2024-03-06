// Copyright (c) 2022-2024 Manuel Schneider

#pragma once
#include <albert/triggerqueryhandler.h>
#include <albert/extensionplugin.h>

class Plugin : public albert::ExtensionPlugin,
               public albert::TriggerQueryHandler
{
    ALBERT_PLUGIN
public:
    QString defaultTrigger() const override;
    void handleTriggerQuery(albert::Query*) override;

    const QStringList icon_urls = {"qfip:/System/Applications/Dictionary.app"};
};
