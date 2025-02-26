// Copyright (c) 2022-2024 Manuel Schneider

#pragma once
#include <QString>
#include <albert/extensionplugin.h>
#include <albert/fallbackhandler.h>
#include <albert/globalqueryhandler.h>

struct SearchEngine
{
    QString id;
    QString name;
    QString trigger;
    QString iconUrl;
    QString url;
    bool fallback;
};

class Plugin : public albert::ExtensionPlugin,
               public albert::GlobalQueryHandler,
               public albert::FallbackHandler

{
    ALBERT_PLUGIN

public:
    Plugin();
    const std::vector<SearchEngine>& engines() const;
    void setEngines(std::vector<SearchEngine> engines);
    void restoreDefaultEngines();

private:
    std::vector<albert::RankItem> handleGlobalQuery(const albert::Query &) override;
    std::vector<std::shared_ptr<albert::Item>> fallbacks(const QString &) const override;
    QWidget *buildConfigWidget() override;

    std::vector<SearchEngine> searchEngines_;

signals:
    void enginesChanged(const std::vector<SearchEngine> &engines);

};
