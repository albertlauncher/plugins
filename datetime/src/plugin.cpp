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

vector<RankItem> Plugin::handleQuery(const Query &query) const
{
    vector<RankItem> r;
    const auto &s = query.string();
    QDateTime dt = QDateTime::currentDateTime();

    if (QString t("date"); t.startsWith(query.string())){
        auto ldate = QLocale::system().toString(dt.date(), QLocale::LongFormat);
        auto sdate = QLocale::system().toString(dt.date(), QLocale::ShortFormat);
        r.emplace_back(
            StandardItem::make(
                t, ldate, "Current date", {":datetime"},
                {
                    {"lcp", "Copy", [=](){setClipboardText(ldate);} },
                    {"scp", "Copy short form", [=](){setClipboardText(sdate);} }
                }
            ), score(s, t)
        );
    }

    if (QString t("time"); t.startsWith(query.string())){
        auto ltime = QLocale::system().toString(dt.time(), QLocale::LongFormat);
        auto stime = QLocale::system().toString(dt.time(), QLocale::ShortFormat);
        r.emplace_back(
            StandardItem::make(
                t, stime, "Current time", {":datetime"},
                {
                    {"scp", "Copy", [=](){setClipboardText(stime);} },
                    {"lcp", "Copy long form", [=](){setClipboardText(ltime);} }
                }
            ), score(s, t)
        );
    }

    if (QString t("unix"); t.startsWith(query.string())){
        auto unixtime = QString::number(QDateTime::currentSecsSinceEpoch());
        r.emplace_back(
            StandardItem::make(
                t, unixtime, "Current seconds since epoch (unixtime)", {":datetime"},
                {{ "copy", "Copy", [=](){setClipboardText(unixtime);} }}
            ), score(s, t)
        );
    }

    if (QString t("utc"); t.startsWith(query.string())){
        auto l = QLocale::system().toString(dt.toUTC(), QLocale::LongFormat);
        auto s = QLocale::system().toString(dt.toUTC(), QLocale::ShortFormat);
        r.emplace_back(
            StandardItem::make(
                t, s, "Current UTC date and time", {":datetime"},
                {
                        {"scp", "Copy", [=](){setClipboardText(s);} },
                        {"lcp", "Copy long form", [=](){setClipboardText(l);} }
                }
            ), score(s, t)
        );
    }

    bool isNumber;
    ulong unixtime = s.toULong(&isNumber);
    if (isNumber){
        auto dt = QLocale::system().toString(QDateTime::fromSecsSinceEpoch(unixtime), QLocale::LongFormat);
        r.emplace_back(
            StandardItem::make(
                "fromunix", dt, "Datetime from unix time", {":datetime"},
                {{ "copy", "Copy", [=](){setClipboardText(dt);} }}
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

void TimeZoneHandler::handleQuery(QueryHandler::Query &query) const
{
    auto loc = QLocale::system();
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
                    QLocale::system().toString(dt, QLocale::ShortFormat),
                    tz_info,
                    {":datetime"},
                    {
                            {"scp", "Copy short form",
                                    [=]() { setClipboardText(QLocale::system().toString(dt, QLocale::ShortFormat)); }},
                            {"lcp", "Copy long form",
                                    [=]() { setClipboardText(QLocale::system().toString(dt, QLocale::LongFormat)); }}
                    }
            ));
        }
    }
}
