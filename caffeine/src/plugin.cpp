// Copyright (c) 2017-2024 Manuel Schneider

#include "plugin.h"
#include "ui_configwidget.h"
#include <QStandardPaths>
#include <albert/logging.h>
#include <albert/logging.h>
#include <albert/matcher.h>
#include <albert/standarditem.h>
ALBERT_LOGGING_CATEGORY("caffeine")
using namespace albert;
using namespace std;

const QStringList Plugin::icon_urls = {"gen:?text=☕️"};

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

Plugin::Plugin()
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
        throw runtime_error(process.program().toStdString() + " not found");

    restore_default_timeout(settings());

    timer.setSingleShot(true);

    QObject::connect(&timer, &QTimer::timeout, [this]{ stop(); });
    QObject::connect(&notification, &Notification::activated, [this]{ stop(); });

    notification.setTitle(name());
}

Plugin::~Plugin()
{
    stop();
}

QWidget* Plugin::buildConfigWidget()
{
    auto *w = new QWidget;
    Ui::ConfigWidget ui;
    ui.setupUi(w);

    ALBERT_PROPERTY_CONNECT_SPINBOX(this, default_timeout, ui.spinBox_minutes)

    // ui.verticalLayout->addStretch();

    return w;
}

QString Plugin::synopsis() const { return tr("[h:]min"); }

void Plugin::setTrigger(const QString &t) { trigger = t; }

QString Plugin::defaultTrigger() const { return tr("si ", "abbr of name()"); }

void Plugin::handleTriggerQuery(Query *query){ query->add(makeItem(query->string())); }

vector<RankItem> Plugin::handleGlobalQuery(const Query *query)
{
    Matcher matcher(query->string());
    auto item = makeItem();
    vector<RankItem> r;
    if (auto m = max({matcher.match(trigger),
                      matcher.match(item->text()),  // name
                      matcher.match(item->subtext())});  // action
        m)
        r.emplace_back(item, m);
    return r;
}

vector<shared_ptr<Item>> Plugin::handleEmptyQuery(const Query *)
{
    vector<shared_ptr<Item>> results;

    if (process.state() == QProcess::Running)
        results.emplace_back(makeItem());

    return results;
}

void Plugin::start(uint minutes)
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

void Plugin::stop()
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

static QString makeActionName(uint minutes)
{
    if (minutes)
        return Plugin::tr("Inhibit sleep for %1").arg(durationString(minutes));
    else
        return Plugin::tr("Inhibit sleep");
}

std::shared_ptr<Item> Plugin::makeItem(const QString &query_string)
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
            auto action_name = makeActionName(default_timeout_);
            return StandardItem::make(
                id(), name(), action_name, defaultTrigger(), icon_urls,
                {{ id(), action_name, [this]{ start(default_timeout_); } }}
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
                auto action_name = makeActionName(default_timeout_);
                return StandardItem::make(
                    id(), name(), t.arg(action_name), defaultTrigger(), icon_urls,
                    {{ id(), action_name, [this]{ start(default_timeout_); } }}
                    );
            }
        }
    }
}
