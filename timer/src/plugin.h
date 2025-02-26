// Copyright (c) 2024 Manuel Schneider

#pragma once
#include <QTimer>
#include <albert/extensionplugin.h>
#include <albert/globalqueryhandler.h>
#include <albert/notification.h>
#include <list>


class Timer : public QTimer
{
public:

    Timer(const QString &name, int interval);
    void onTimeout();
    const uint64_t end;
    albert::Notification notification;

};


class Plugin : public albert::ExtensionPlugin,
               public albert::GlobalQueryHandler
{
    ALBERT_PLUGIN

public:

    QString defaultTrigger() const override;
    QString synopsis(const QString &) const override;
    std::vector<albert::RankItem> handleGlobalQuery(const albert::Query &) override;
    std::vector<std::shared_ptr<albert::Item>> handleEmptyQuery() override;

private:

    std::shared_ptr<albert::Item> makeSetTimerItem(uint dur, const QString &name);
    std::shared_ptr<albert::Item> makeTimerItem(Timer&);
    void startTimer(const QString &name, uint seconds);
    void removeTimer(Timer*);

    const QStringList icon_urls{"gen:?text=⏲️"};
    std::list<Timer> timers_;
    uint timer_counter_ = 0;

};
