// Copyright (c) 2024-2024 Manuel Schneider

#pragma once
#include <albert/globalqueryhandler.h>
#include <albert/extensionplugin.h>

class Plugin : public albert::ExtensionPlugin,
               public albert::GlobalQueryHandler
{
    ALBERT_PLUGIN
public:
    Plugin();
    ~Plugin();
    QString defaultTrigger() const override;
    bool supportsFuzzyMatching() const override;
    void setFuzzyMatching(bool enabled) override;
    std::vector<albert::RankItem> handleGlobalQuery(const albert::Query &) override;

    class Private;
    std::unique_ptr<Private> d;
};
