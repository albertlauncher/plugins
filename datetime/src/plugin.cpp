// Copyright (c) 2023 Manuel Schneider

#include "plugin.h"
using namespace std;
using namespace albert;

static RankItem::Score score(const QString &s, const QString &t)
{
    return (double)s.size()/t.size()*RankItem::MAX_SCORE;
}

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
                    {"copy", "Copy", [=](){setClipboardText(ldate);} },
                    {"copy", "Copy short form", [=](){setClipboardText(sdate);} }
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
                    {"copy", "Copy", [=](){setClipboardText(stime);} },
                    {"copy", "Copy long form", [=](){setClipboardText(ltime);} }
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
        auto utc = QLocale::system().toString(dt.toUTC(), QLocale::LongFormat);
        r.emplace_back(
            StandardItem::make(
                t, utc, "Current UTC date and time", {":datetime"},
                {{ "copy", "Copy", [=](){setClipboardText(utc);} }}
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
