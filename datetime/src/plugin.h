// Copyright (c) 2023-2024 Manuel Schneider

#pragma once
#include "albert/query/globalqueryhandler.h"
#include "albert/util/extensionplugin.h"
#include "albert/util/properties.h"
class QLocale;


class TimeZoneHandler : public albert::TriggerQueryHandler
{
public:
    QString id() const override;
    QString name() const override;
    QString description() const override;
    QString synopsis() const override;
    QString defaultTrigger() const override;
    void handleTriggerQuery(albert::Query*) override;
};


class Plugin : public albert::ExtensionPlugin,
               public albert::GlobalQueryHandler
{
    Q_OBJECT
    ALBERT_PLUGIN
    ALBERT_PLUGIN_PROPERTY(bool, show_date_on_empty_query, false)

public:
    Plugin();

    QWidget *buildConfigWidget() override;
    void initialize(albert::ExtensionRegistry &registry, std::map<QString,PluginInstance*> dependencies) override;
    void finalize(albert::ExtensionRegistry &registry) override;
    QString synopsis() const override;
    std::vector<std::shared_ptr<albert::Item>> handleEmptyQuery(const albert::Query*) const override;
    std::vector<albert::RankItem> handleGlobalQuery(const albert::Query*) const override;

    const QString tr_copy;
    const QString tr_copy_short;
    const QString tr_copy_long;
    const QString tr_date;
    const QString tr_time;
    const QString tr_d_unix;
    const QString tr_d_utc;

    static const QStringList icon_urls;

private:
    TimeZoneHandler tzh;

};
