// Copyright (c) 2023 Manuel Schneider

#include "plugin.h"
using namespace std;
using namespace albert;

static RankItem::Score score(const QString &s, const QString &t)
{
    return (double)s.size()/t.size()*RankItem::MAX_SCORE;
}

Plugin::Plugin() { registry().add(&tzh); }

Plugin::~Plugin() { registry().remove(&tzh); }

vector<RankItem> Plugin::handleGlobalQuery(const GlobalQuery &query) const
{
    vector<RankItem> r;
    const auto &s = query.string();
    QDateTime dt = QDateTime::currentDateTime();
    QLocale loc;

    if (QString t("date"); t.startsWith(query.string())){
        auto ls = loc.toString(dt.date(), QLocale::LongFormat);
        auto ss = loc.toString(dt.date(), QLocale::ShortFormat);
        r.emplace_back(
            StandardItem::make(
                    t, ls, "Current date", {":datetime"},
                    {
                    {"lcp", "Copy", [=](){setClipboardText(ls);} },
                    {"scp", "Copy short form", [=](){setClipboardText(ss);} }
                }
            ),
            score(s, t)
        );
    }

    if (QString t("time"); t.startsWith(query.string())){
        auto ls = loc.toString(dt.time(), QLocale::LongFormat);
        auto ss = loc.toString(dt.time(), QLocale::ShortFormat);
        r.emplace_back(
            StandardItem::make(
                    t, ss, "Current time", {":datetime"},
                    {
                    {"scp", "Copy", [=](){setClipboardText(ss);} },
                    {"lcp", "Copy long form", [=](){setClipboardText(ls);} }
                }
            ),
            score(s, t)
        );
    }

    if (QString t("unix"); t.startsWith(query.string())){
        auto unixtime = QString::number(QDateTime::currentSecsSinceEpoch());
        r.emplace_back(
            StandardItem::make(
                t, unixtime, "Current seconds since epoch (unixtime)", {":datetime"},
                {{ "copy", "Copy", [=](){setClipboardText(unixtime);} }}
            ),
            score(s, t)
        );
    }

    if (QString t("utc"); t.startsWith(query.string())){
        auto ls = loc.toString(dt.toUTC(), QLocale::LongFormat);
        auto ss = loc.toString(dt.toUTC(), QLocale::ShortFormat);
        r.emplace_back(
            StandardItem::make(
                t, ss, "Current UTC date and time", {":datetime"},
                {
                        {"scp", "Copy", [=](){setClipboardText(ss);} },
                        {"lcp", "Copy long form", [=](){setClipboardText(ls);} }
                }
            ),
            score(s, t)
        );
    }

    bool isNumber;
    ulong unixtime = s.toULong(&isNumber);
    if (isNumber){
        auto ls = loc.toString(QDateTime::fromSecsSinceEpoch(unixtime), QLocale::LongFormat);
        r.emplace_back(
            StandardItem::make(
                "fromunix", ls, "Datetime from unix time", {":datetime"},
                {{ "copy", "Copy", [=](){setClipboardText(ls);} }}
            ), 0
        );
    }

    return r;
}

QString TimeZoneHandler::id() const { return QStringLiteral("timezones"); }

QString TimeZoneHandler::name() const { return QStringLiteral("TimeZones"); }

QString TimeZoneHandler::description() const { return QStringLiteral("Get times around the world"); }

QString TimeZoneHandler::synopsis() const { return QStringLiteral("<tz id/name>"); }

QString TimeZoneHandler::defaultTrigger() const { return QStringLiteral("tz "); }

void TimeZoneHandler::handleTriggerQuery(TriggerQuery &query) const
{
    QLocale loc;
    auto utc = QDateTime::currentDateTimeUtc();

    for (auto &tz_id_barray: QTimeZone::availableTimeZoneIds()){
        if (!query.isValid()) return;

        auto tz = QTimeZone(tz_id_barray);
        auto tz_id = QString::fromLocal8Bit(tz_id_barray);
        auto dt = utc.toTimeZone(tz);
        auto tz_info = QString("%1, %2").arg(tz_id, tz.displayName(QTimeZone::StandardTime, QTimeZone::LongName, loc));

        if (tz_info.contains(query.string(), Qt::CaseInsensitive)) {
            query.add(StandardItem::make(
                    tz_id,
                    loc.toString(dt, QLocale::ShortFormat),
                    tz_info,
                    {":datetime"},
                    {
                            {"scp", "Copy short form",
                                    [=]() { setClipboardText(QLocale().toString(dt, QLocale::ShortFormat)); }},
                            {"lcp", "Copy long form",
                                    [=]() { setClipboardText(QLocale().toString(dt, QLocale::LongFormat)); }}
                    }
            ));
        }
    }
}
