// Copyright (c) 2023 Manuel Schneider

#include "albert/albert.h"
#include "albert/extension/queryhandler/standarditem.h"
#include "plugin.h"
#include "albert/extensionregistry.h"
#include "ui_configwidget.h"
#include <QDateTime>
#include <QLocale>
#include <QTimeZone>
#include <QWidget>
using namespace albert;
using namespace std;

const QStringList Plugin::icon_urls{":datetime"};

void Plugin::initialize(ExtensionRegistry &registry, map<QString,PluginInstance*>)
{
    registry.registerExtension(this);
    registry.registerExtension(&tzh);
}

void Plugin::finalize(albert::ExtensionRegistry &registry)
{
    registry.deregisterExtension(this);
    registry.deregisterExtension(&tzh);
}

QWidget *Plugin::buildConfigWidget()
{
    auto *w = new QWidget();
    Ui::ConfigWidget ui;
    ui.setupUi(w);
    return w;
}

vector<RankItem> Plugin::handleGlobalQuery(const GlobalQuery *query) const
{
    vector<RankItem> r;
    const auto &s = query->string();
    QDateTime dt = QDateTime::currentDateTime();
    QLocale loc;

    static const auto tr_c = tr("Copy");
    static const auto tr_cs = tr("Copy short form");
    static const auto tr_cl = tr("Copy long form");

    if (static const auto tr_d = tr("Date"); tr_d.startsWith(query->string(), Qt::CaseInsensitive))
    {
        auto ls = loc.toString(dt.date(), QLocale::LongFormat);
        auto ss = loc.toString(dt.date(), QLocale::ShortFormat);

        r.emplace_back(
            StandardItem::make(
                QStringLiteral("d"), ls, tr_d, icon_urls,
                {
                    {QStringLiteral("lcp"), tr_c, [=](){setClipboardText(ls);} },
                    {QStringLiteral("scp"), tr_cs, [=](){setClipboardText(ss);} }
                }
            ),
            (float)s.size()/tr_d.size()
        );
    }


    if (static const auto tr_t = tr("Time"); tr_t.startsWith(query->string(), Qt::CaseInsensitive))
    {
        auto ls = loc.toString(dt.time(), QLocale::LongFormat);
        auto ss = loc.toString(dt.time(), QLocale::ShortFormat);

        r.emplace_back(
            StandardItem::make(
                QStringLiteral("t"), ss, tr_t, icon_urls,
                {
                    {QStringLiteral("scp"), tr_c, [=](){ setClipboardText(ss); }},
                    {QStringLiteral("lcp"), tr_cl, [=](){ setClipboardText(ls); }}
                }
            ),
            (float)s.size()/tr_t.size()
        );
    }


    if (static const QString u("unix"); u.startsWith(query->string(), Qt::CaseInsensitive))
    {
        static const auto tr_d = tr("Seconds since epoch (unixtime)");

        auto unixtime = QString::number(QDateTime::currentSecsSinceEpoch());

        r.emplace_back(
            StandardItem::make(
                u, unixtime, tr_d, icon_urls,
                {
                    {QStringLiteral("c"), tr_c, [=](){ setClipboardText(unixtime); }}
                }
            ),
            (float)s.size()/u.size()
        );
    }

    if (static const QString u("utc"); u.startsWith(query->string(), Qt::CaseInsensitive))
    {
        static const auto tr_d = tr("UTC date and time");

        auto ls = loc.toString(dt.toUTC(), QLocale::LongFormat);
        auto ss = loc.toString(dt.toUTC(), QLocale::ShortFormat);

        r.emplace_back(
            StandardItem::make(
                u, ss, tr_d, icon_urls,
                {
                    {QStringLiteral("scp"), tr_c, [=](){ setClipboardText(ss); }},
                    {QStringLiteral("lcp"), tr_cl, [=](){ setClipboardText(ls); }}
                }
            ),
            (float)s.size()/u.size()
        );
    }

    bool isNumber;
    ulong unixtime = s.toULong(&isNumber);
    if (isNumber)
    {
        static const auto tr_d = tr("Date and time from unix time");

        auto ls = loc.toString(QDateTime::fromSecsSinceEpoch(unixtime), QLocale::LongFormat);

        r.emplace_back(
            StandardItem::make(
                QStringLiteral("fromunix"), ls, tr_d, icon_urls,
                {
                    {QStringLiteral("c"), tr_c, [=](){ setClipboardText(ls); }}
                }
            ),
            0
        );
    }

    return r;
}

QString TimeZoneHandler::id() const
{ return QStringLiteral("timezones"); }

QString TimeZoneHandler::name() const
{
    static const auto tr = QCoreApplication::translate("TimeZoneHandler", "TimeZones");
    return tr;
}

QString TimeZoneHandler::description() const
{
    static const auto tr = QCoreApplication::translate("TimeZoneHandler", "Date and time of any timezone");
    return tr;
}

QString TimeZoneHandler::synopsis() const
{ return QStringLiteral("<tz id/name>"); }

QString TimeZoneHandler::defaultTrigger() const
{ return QStringLiteral("tz "); }

void TimeZoneHandler::handleTriggerQuery(TriggerQuery *query) const
{
    QLocale loc;
    auto utc = QDateTime::currentDateTimeUtc();

    for (auto &tz_id_barray: QTimeZone::availableTimeZoneIds())
    {
        if (!query->isValid()) return;

        auto tz = QTimeZone(tz_id_barray);
        auto tz_id = QString::fromLocal8Bit(tz_id_barray).replace("_", " ");
        auto dt = utc.toTimeZone(tz);
        auto tz_info = QString("%1, %2").arg(tz_id, tz.displayName(QTimeZone::StandardTime, QTimeZone::LongName, loc));

        if (tz_info.contains(query->string(), Qt::CaseInsensitive))
        {
            static const auto tr_s = QCoreApplication::translate("TimeZoneHandler", "Copy short form");
            static const auto tr_l = QCoreApplication::translate("TimeZoneHandler", "Copy long form");

            query->add(
                StandardItem::make(
                    tz_id,
                    loc.toString(dt, QLocale::ShortFormat),
                    tz_info,
                    Plugin::icon_urls,
                    {
                        {
                            QStringLiteral("scp"), tr_s,
                            [=]() { setClipboardText(QLocale().toString(dt, QLocale::ShortFormat)); }
                        },
                        {
                            QStringLiteral("lcp"), tr_l,
                            [=]() { setClipboardText(QLocale().toString(dt, QLocale::LongFormat)); }
                        }
                    }
                )
            );
        }
    }
}
