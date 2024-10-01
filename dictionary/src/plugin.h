// Copyright (c) 2022-2024 Manuel Schneider

#pragma once
#include <albert/extensionplugin.h>
#include <albert/fallbackhandler.h>
#include <albert/triggerqueryhandler.h>

class Plugin : public albert::ExtensionPlugin,
               public albert::TriggerQueryHandler,
               public albert::FallbackHandler
{
    ALBERT_PLUGIN
public:
    Plugin();
    QString defaultTrigger() const override;
    void handleTriggerQuery(albert::Query*q) override;
    std::vector<std::shared_ptr<albert::Item> > fallbacks(const QString &) const override;

private:
    struct {
        QString dict;
        QString lookup;
        QString lookup_arg;
    } tr_;
};
