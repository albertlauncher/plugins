// Copyright (c) 2017-2024 Manuel Schneider

#include "inhibitsleep.h"
#include <QStandardPaths>
#include <albert/logging.h>
#include <albert/matcher.h>
#include <albert/standarditem.h>

using namespace albert;
using namespace std;

static QString durationString(uint minutes)
{
    uint hours = minutes / 60;
    minutes -= hours * 60;
    return QStringLiteral("%1:%2")
            .arg(hours, 2, 10, QChar('0'))
            .arg(minutes, 2, 10, QChar('0'));
}

static optional<uint> parseDurationStringToMinutes(const QString &s)
{
    auto fields = s.split(':');
    if (fields.size() > 2)
        return 0;

    bool ok;
    uint minutes = 0;
    uint scalar = 1;
    for (auto it = fields.rbegin(); it != fields.rend(); ++it)
    {
        int val = 0;
        if (!it->isEmpty()) {
            val = it->toInt(&ok);
            if (!ok || val < 0)
                return {};
        }
        minutes += val * scalar;
        scalar *= 60;
    }
    return minutes;
}

const QStringList InhibitSleep::icon_urls = {"gen:?text=☕️"};

InhibitSleep::InhibitSleep()
{

#if defined(Q_OS_MAC)
    process.setProgram("caffeinate");
    process.setArguments({"-d", "-i"});
#elif defined(Q_OS_UNIX)
    process.setProgram("systemd-inhibit");
    process.setArguments({"--what=idle:sleep",
                          QString("--who=%1").arg(QCoreApplication::applicationName()),
                          "--why=User",
                          "sleep",
                          "infinity"});
#else
    throw std::runtime_error("Unsupported OS");
#endif

    if (auto e = QStandardPaths::findExecutable(process.program()); e.isEmpty())
        throw runtime_error(e.toStdString() + " not found");

    timer.setSingleShot(true);

    QObject::connect(&timer, &QTimer::timeout, [this]{ stop(); });
    QObject::connect(&notification, &Notification::activated, [this]{ stop(); });

    notification.setTitle(name());

}

InhibitSleep::~InhibitSleep()
{
    stop();
}

QString InhibitSleep::id() const { return QStringLiteral("inhibit_sleep"); }

QString InhibitSleep::name() const { return tr("Sleep inhibition"); }

QString InhibitSleep::description() const { return tr("Prevent the system from sleeping"); }

bool InhibitSleep::allowTriggerRemap() const { return false; }

void InhibitSleep::setTrigger(const QString &t) { trigger = t; }

QString InhibitSleep::defaultTrigger() const { return tr("si ", "abbr of name()"); }

QString InhibitSleep::synopsis() const { return tr("[h:]min"); }

void InhibitSleep::handleTriggerQuery(Query *query){ query->add(makeItem(query->string())); }

vector<RankItem> InhibitSleep::handleGlobalQuery(const Query *query)
{
    vector<RankItem> r;

    Matcher matcher(query->string());
    if (auto m = matcher.match(name()))
        r.emplace_back(makeItem(), m);

    return r;
}

vector<shared_ptr<Item>> InhibitSleep::handleEmptyQuery(const Query *)
{
    vector<shared_ptr<Item>> results;

    if (process.state() == QProcess::Running)
        results.emplace_back(makeItem());

    return results;
}

QString InhibitSleep::makeActionName(uint minutes)
{
    if (minutes)
        return tr("Inhibit sleep for %1").arg(durationString(minutes));
    else
        return tr("Inhibit sleep");
}

void InhibitSleep::start(uint minutes)
{
    stop();

    process.start();
    if (!process.waitForStarted(1000) || process.state() != QProcess::Running)
        WARN << "Sleep inhibition failed" << process.errorString();
    else
    {
        INFO << "Sleep inhibition started";

        notification.setTitle(tr("Albert inhibits sleep"));
        notification.setText(tr("Click to stop the sleep inhibition"));
        notification.send();

        if (minutes > 0)
            timer.start(minutes * 60 * 1000);
    }
}

void InhibitSleep::stop()
{
    if (process.state() == QProcess::Running)
    {
        INFO << "Sleep inhibition stoppped";
        notification.dismiss();
        process.kill();
        process.waitForFinished();
        timer.stop();
    }
}

std::shared_ptr<Item> InhibitSleep::makeItem(const QString &query_string)
{
    if (query_string.isEmpty())
    {
        if (process.state() == QProcess::Running)
        {
            // Stop
            auto action_name = tr("Stop sleep inhibition");
            return StandardItem::make(
                id(), name(), action_name, defaultTrigger(), icon_urls,
                {{ id(), action_name, [this]{ stop(); } }}
            );
        }
        else
        {
            // Start default
            auto action_name = makeActionName(default_timeout);
            return StandardItem::make(
                id(), name(), action_name, defaultTrigger(), icon_urls,
                {{ id(), action_name, [this]{ start(default_timeout); } }}
            );
        }
    }
    else
    {
        auto minutes = parseDurationStringToMinutes(query_string);
        if (minutes)
        {
            if (process.state() == QProcess::Running)
            {
                // Restart with given timeout
                auto action_name = makeActionName(minutes.value());
                return StandardItem::make(
                    id(), name(), action_name, icon_urls,
                    {{ id(), action_name, [this, m=minutes.value()]{ start(m); } }}
                );
            }
            else
            {
                // Start with given timeout
                auto action_name = makeActionName(minutes.value());
                return StandardItem::make(
                    id(), name(), action_name, icon_urls,
                    {{ id(), action_name, [this, m=minutes.value()]{ start(m); } }}
                );
            }
        }
        else
        {
            // Invalid query
            auto t = tr("Invalid interval. %1.");

            if (process.state() == QProcess::Running)
            {
                // Stop
                auto action_name = tr("Stop sleep inhibition");
                return StandardItem::make(
                    id(), name(), t.arg(action_name), defaultTrigger(), icon_urls,
                    {{ id(), action_name, [this]{ stop(); } }}
                );
            }
            else
            {
                // Start indefinitely
                auto action_name = makeActionName(default_timeout);
                return StandardItem::make(
                    id(), name(), t.arg(action_name), defaultTrigger(), icon_urls,
                    {{ id(), action_name, [this]{ start(default_timeout); } }}
                );
            }
        }
    }
}
