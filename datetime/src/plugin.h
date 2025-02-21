// Copyright (c) 2023-2024 Manuel Schneider

#pragma once
#include <albert/extensionplugin.h>
#include <albert/globalqueryhandler.h>
#include <albert/property.h>
#include <QObject>

class Plugin : public albert::ExtensionPlugin,
               public albert::GlobalQueryHandler
{
    ALBERT_PLUGIN
    ALBERT_PLUGIN_PROPERTY(bool, show_date_on_empty_query, false)

public:
    Plugin();

    QWidget *buildConfigWidget() override;
    QString synopsis(const QString &) const override;
    std::vector<albert::RankItem> handleGlobalQuery(const albert::Query &) override;
    std::vector<std::shared_ptr<albert::Item>> handleEmptyQuery() override;

    QStringList icon_urls{":datetime"};

    const QString tr_time;
    const QString tr_date;
    const QString tr_unix;
    const QString utc;

};
