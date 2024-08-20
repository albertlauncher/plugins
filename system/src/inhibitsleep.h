// Copyright (c) 2017-2024 Manuel Schneider

#pragma once
#include <QCoreApplication>
#include <QProcess>
#include <QTimer>
#include <albert/extensionplugin.h>
#include <albert/indexqueryhandler.h>
#include <albert/notification.h>


class InhibitSleep : public albert::GlobalQueryHandler
{
    Q_DECLARE_TR_FUNCTIONS(InhibitSleep)

    QString id() const override;
    QString name() const override final;
    QString description() const override;
    void setTrigger(const QString&) override;
    QString defaultTrigger() const override;
    QString synopsis() const override;
    void handleTriggerQuery(albert::Query *) override;
    std::vector<albert::RankItem> handleGlobalQuery(const albert::Query *) override;
    std::vector<std::shared_ptr<albert::Item>> handleEmptyQuery(const albert::Query *) override;

    std::shared_ptr<albert::Item> makeItem(const QString &query_string = {});
    QString makeActionName(uint minutes);
    void start(uint minutes);
    void stop();

    QProcess process;
    QTimer timer;
    albert::Notification notification;
    QStringList commandline;
    static const QStringList icon_urls;
    QString trigger;

public:

    InhibitSleep();
    ~InhibitSleep();

    uint default_timeout;

};
