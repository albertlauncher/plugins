// Copyright (c) 2023-2024 Manuel Schneider

#pragma once
#include <albert/extensionplugin.h>
#include <albert/triggerqueryhandler.h>

namespace albert::timezones
{
class Plugin : public albert::ExtensionPlugin,
               public albert::TriggerQueryHandler
{
    ALBERT_PLUGIN

public:

    QString defaultTrigger() const override;
    void handleTriggerQuery(albert::Query *) override;

    QStringList icon_urls{":timezones"};

};

}
