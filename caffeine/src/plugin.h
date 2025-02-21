// Copyright (c) 2017-2024 Manuel Schneider

#pragma once
#include <QProcess>
#include <QStringList>
#include <QTimer>
#include <albert/extensionplugin.h>
#include <albert/globalqueryhandler.h>
#include <albert/notification.h>
#include <albert/property.h>



class Plugin : public albert::ExtensionPlugin,
               public albert::GlobalQueryHandler
{
    ALBERT_PLUGIN

public:

    Plugin();
    ~Plugin();

    QWidget *buildConfigWidget() override;

    QString synopsis(const QString &) const override;
    void setTrigger(const QString&) override;
    QString defaultTrigger() const override;
    void handleTriggerQuery(albert::Query &) override;
    std::vector<albert::RankItem> handleGlobalQuery(const albert::Query &) override;
    std::vector<std::shared_ptr<albert::Item>> handleEmptyQuery() override;

private:
    std::shared_ptr<albert::Item> makeItem(const QString &query_string = {});
    void start(uint minutes);
    void stop();

    QProcess process;
    QTimer timer;
    albert::Notification notification;
    QStringList commandline;
    static const QStringList icon_urls;
    QString trigger;

    ALBERT_PLUGIN_PROPERTY(uint, default_timeout, 60)

};
