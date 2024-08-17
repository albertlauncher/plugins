// Copyright (c) 2024-2024 Manuel Schneider

#include "plugin.h"
#include <QDateTime>
#include <QLocale>
#include <albert/matcher.h>
#include <albert/standarditem.h>
#include <albert/util.h>
using namespace albert::timer;
using namespace albert;
using namespace std;


Timer::Timer(const QString &name, int interval):
    end(QDateTime::currentSecsSinceEpoch() + interval)
{
    setObjectName(name);
    setSingleShot(true);
    start(interval * 1000);
    connect(this, &Timer::timeout, this, &Timer::onTimeout);
}

void Timer::onTimeout()
{
    notification.setTitle(tr("Timer %1").arg(objectName()));
    auto dts = QDateTime::currentDateTime().toString("hh:mm:ss");
    notification.setText(tr("Timed out %1").arg(dts));
    notification.send();
}

QString Plugin::defaultTrigger() const
{ return tr("timer ", "The trigger. Lowercase."); }

QString Plugin::synopsis() const
{ return tr("[[h:]min:]s [name]"); }

static QString durationString(uint seconds)
{
    uint hours = seconds / 3600;
    seconds -= hours * 3600;
    uint minutes = seconds / 60;
    seconds -= minutes * 60;

    return QStringLiteral("%1:%2:%3")
            .arg(hours, 2, 10, QChar('0'))
            .arg(minutes, 2, 10, QChar('0'))
            .arg(seconds, 2, 10, QChar('0'));
}

static uint parseDurationString(const QString &s)
{
    if (!s.contains(':'))
        return 0;

    auto fields = s.split(':');
    if (fields.size() > 3)
        return 0;

    bool ok;
    uint dur = 0;
    uint scalar = 1;
    for (auto it = fields.rbegin(); it != fields.rend(); ++it)
    {
        int val = 0;
        if (!it->isEmpty()) {
            val = it->toInt(&ok);
            if (!ok || val < 0)
                return {};
        }
        dur += val * scalar;
        scalar *= 60;
    }
    return dur;
}

vector<RankItem> Plugin::handleGlobalQuery(const Query *query)
{
    if (!query->isValid())
        return {};

    Matcher matcher(query->string());
    vector<RankItem> r;

    // List matching timers
    for (auto &timer: timers_)
        if(auto m = matcher.match(timer.objectName()); m)
            r.emplace_back(makeTimerItem(timer), m);

    // Add new timer item
    auto dur = parseDurationString(query->string().section(' ', 0, 0, QString::SectionSkipEmpty));
    if (dur > 0)
    {
        auto name = query->string().section(' ', 1, -1, QString::SectionSkipEmpty);
        if (name.isEmpty())
            name = QString("#%1").arg(timer_counter_);

        r.emplace_back(
            StandardItem::make(
                QStringLiteral("timer"),
                tr("Set timer: %1").arg(name),
                durationString(dur),
                {QStringLiteral(":datetime")},
                {
                    {
                        QStringLiteral("set"), tr("Start", "Action verb form"),
                        [=, this]{ startTimer(name, dur); }
                    }
                }
            ),
            1.0
        );
    }

    return r;
}

vector<shared_ptr<Item>> Plugin::handleEmptyQuery(const Query *)
{
    vector<shared_ptr<Item>> results;
    for (auto &timer: timers_)
        results.emplace_back(makeTimerItem(timer));
    return results;
}

std::shared_ptr<Item> Plugin::makeTimerItem(Timer &t)
{
    return StandardItem::make(
        QStringLiteral("timer_item"),
        tr("Timer: %1").arg(t.objectName()),
        (t.isActive() ? tr("%1, Times out %2") : tr("%1, Timed out %2"))
                .arg(durationString(t.interval() / 1000),
                     QDateTime::fromSecsSinceEpoch(t.end).toString("hh:mm:ss")),
        {QStringLiteral(":datetime")},
        {
            {
                QStringLiteral("rem"), tr("Remove", "Action verb form"),
                [t=&t, this] { removeTimer(t); }
            }
        }
    );
}

void Plugin::startTimer(const QString &name, uint seconds)
{
    ++timer_counter_;
    auto &timer = timers_.emplace_front(name, seconds);
    QObject::connect(&timer.notification, &Notification::activated,
                     &timer.notification, [t=&timer, this]{ removeTimer(t); });
}

void Plugin::removeTimer(Timer *t)
{
    if (auto it = std::find_if(timers_.begin(), timers_.end(),
                               [&](const auto& o) {return t == &o;});
        it != timers_.end())
        timers_.erase(it);
}
