// Copyright (c) 2022 Manuel Schneider

#pragma once
#include "albert/extension/queryhandler/fallbackprovider.h"
#include "albert/extension/queryhandler/globalqueryhandler.h"
#include "albert/plugin.h"
#include <QString>

struct SearchEngine
{
    QString id;
    QString name;
    QString trigger;
    QString iconUrl;
    QString url;
};

class Plugin : public albert::plugin::ExtensionPlugin,
               public albert::GlobalQueryHandler,
               public albert::FallbackHandler

{
    Q_OBJECT ALBERT_PLUGIN
public:
    Plugin();
    const std::vector<SearchEngine>& engines() const;
    void setEngines(std::vector<SearchEngine> engines);
    void restoreDefaultEngines();

private:
    std::vector<albert::RankItem> handleGlobalQuery(const GlobalQuery*) const override;
    std::vector<std::shared_ptr<albert::Item>> fallbacks(const QString &) const override;
    QWidget *buildConfigWidget() override;

    std::vector<SearchEngine> searchEngines_;

signals:
    void enginesChanged(const std::vector<SearchEngine> &engines);

};
