// Copyright (c) 2023-2024 Manuel Schneider

#pragma once
#include <albert/extensionplugin.h>
#include <albert/globalqueryhandler.h>
#include <albert/property.h>
#include <QObject>

namespace albert::datetime
{
class Plugin : public albert::ExtensionPlugin,
               public albert::GlobalQueryHandler
{
    ALBERT_PLUGIN
    ALBERT_PLUGIN_PROPERTY(bool, show_date_on_empty_query, false)

public:

    Plugin();

    QWidget *buildConfigWidget() override;
    QString synopsis() const override;
    std::vector<std::shared_ptr<albert::Item>> handleEmptyQuery(const albert::Query*) const override;
    std::vector<RankItem> handleGlobalQuery(const Query *query) const override;

    QStringList icon_urls{":datetime"};

    const QString tr_time;
    const QString tr_date;
    const QString tr_unix;
    const QString utc;

};

}
