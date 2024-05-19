// Copyright (c) 2024 Manuel Schneider

#pragma once
#include <QTimer>
#include <albert/extensionplugin.h>
#include <albert/globalqueryhandler.h>
#include <albert/notification.h>
#include <list>


namespace albert::timer
{

class Timer : public QTimer
{
public:

    Timer(const QString &name, int interval);
    void onTimeout();
    const uint64_t end;
    Notification notification;

};


class Plugin : public albert::ExtensionPlugin,
               public albert::GlobalQueryHandler
{
    ALBERT_PLUGIN

public:

    QString defaultTrigger() const override;
    QString synopsis() const override;
    std::vector<RankItem> handleGlobalQuery(const Query *) const override;
    std::vector<std::shared_ptr<Item>> handleEmptyQuery(const Query *) const override;

private:

    std::shared_ptr<Item> makeTimerItem(Timer&) const;
    void startTimer(const QString &name, uint seconds) const;
    void removeTimer(Timer*) const;

    const QStringList icon_urls{":timer"};
    mutable std::list<Timer> timers_;
    mutable uint timer_counter_ = 0;

};

}
