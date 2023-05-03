// Copyright (c) 2022 Manuel Schneider

#pragma once
#include <QPointer>
#include "albert.h"
#include "searchengine.h"

class ConfigWidget;


class Plugin final : public albert::ExtensionPlugin,
                     public albert::FallbackHandler,
                     public albert::QueryHandler
{
    Q_OBJECT ALBERT_PLUGIN

public:

    Plugin();

    std::vector<albert::RankItem> handleGlobalQuery(const GlobalQuery&) const override;
    std::vector<std::shared_ptr<albert::Item>> fallbacks(const QString &) const override;
    QWidget *buildConfigWidget() override;

    const std::vector<SearchEngine>& engines() const;
    void setEngines(const std::vector<SearchEngine> &engines);

    void restoreDefaultEngines();

private:

    const QString engines_json;
    QPointer<ConfigWidget> widget_;
    std::vector<SearchEngine> searchEngines_;

signals:

    void enginesChanged(const std::vector<SearchEngine> &engines);

};
