// Copyright (c) 2024 Manuel Schneider

#pragma once
#include <albert/globalqueryhandler.h>
#include <albert/extensionplugin.h>

namespace albert::bluetooth {
class Plugin : public albert::ExtensionPlugin,
               public albert::GlobalQueryHandler
{
    ALBERT_PLUGIN

public:

    Plugin();
    ~Plugin();

    QString defaultTrigger() const override;
    bool supportsFuzzyMatching() const override;
    void setFuzzyMatching(bool) override;
    std::vector<RankItem> handleGlobalQuery(const Query *) const override;

private:

    class Private;
    std::unique_ptr<Private> d;

};
}
