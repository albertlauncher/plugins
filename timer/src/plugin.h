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
    std::vector<RankItem> handleGlobalQuery(const Query *) override;
    std::vector<std::shared_ptr<Item>> handleEmptyQuery(const Query *) override;

private:

    std::shared_ptr<Item> makeSetTimerItem(uint dur, const QString &name);
    std::shared_ptr<Item> makeTimerItem(Timer&);
    void startTimer(const QString &name, uint seconds);
    void removeTimer(Timer*);

    const QStringList icon_urls{"gen:?text=⏲️"};
    std::list<Timer> timers_;
    uint timer_counter_ = 0;

};

}
