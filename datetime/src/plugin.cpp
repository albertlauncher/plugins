// Copyright (c) 2023-2024 Manuel Schneider

#include "plugin.h"
#include "ui_configwidget.h"
#include <QDateTime>
#include <QLocale>
#include <QTimeZone>
#include <QWidget>
#include <albert/albert.h>
#include <albert/standarditem.h>
using namespace albert;
using namespace std;


Plugin::Plugin():
    tr_time(tr("Time")),
    tr_date(tr("Date")),
    tr_unix(tr("Unix time")),
    utc("UTC")
{
    restore_show_date_on_empty_query(settings());
}

QString Plugin::synopsis(const QString &query) const
{
    if (query.isEmpty())
        return QStringLiteral("%1 | %2 | unix | utc | <number>")
            .arg(tr("Date").toLower(), tr("Time").toLower());
    return {};
}

inline static QString tr_copy() { return Plugin::tr("Copy to clipboard"); }

inline static QString tr_copy_with_placeholder() { return Plugin::tr("Copy '%1' to clipboard"); }

vector<RankItem> Plugin::handleGlobalQuery(const Query &query)
{
    vector<RankItem> r;
    const auto &s = query.string();

    if (show_date_on_empty_query_ && s.isEmpty())
    {
        const QLocale loc;
        const auto dt = QDateTime::currentDateTime();

        r.emplace_back(
            StandardItem::make(
                QStringLiteral("dt"),
                loc.toString(dt.time(), QLocale::ShortFormat),
                loc.toString(dt.date(), QLocale::LongFormat),
                icon_urls
            ),
            1.0
        );
    }

    if (tr_time.startsWith(query.string(), Qt::CaseInsensitive))
    {
        const QLocale loc;
        const auto dt = QDateTime::currentDateTime();
        const auto t = loc.toString(dt.time(), QLocale::ShortFormat);

        r.emplace_back(
            StandardItem::make(
                QStringLiteral("t"), t, tr_time, tr_time, icon_urls,
                {
                    {
                        QStringLiteral("c"), tr_copy(),
                        [=]{ setClipboardText(t); }
                    }
                }
            ),
            (double)s.size() / tr_time.size()
        );
    }

    if (tr_date.startsWith(query.string(), Qt::CaseInsensitive))
    {
        const QLocale loc;
        const auto dt = QDateTime::currentDateTime();
        const auto lf = loc.toString(dt.date(), QLocale::LongFormat);
        const auto sf = loc.toString(dt.date(), QLocale::ShortFormat);

        r.emplace_back(
            StandardItem::make(
                QStringLiteral("d"), lf, tr_date, tr_date, icon_urls,
                {
                    {
                        QStringLiteral("cl"), tr_copy(),
                        [=]{ setClipboardText(lf); }
                    },
                    {
                        QStringLiteral("cs"), tr_copy_with_placeholder().arg(sf),
                        [=]{ setClipboardText(sf); }
                    }
                }
            ),
            (double)s.size() / tr_date.size()
        );
    }

    if (tr_unix.startsWith(query.string(), Qt::CaseInsensitive))
    {
        const auto t = QString::number(QDateTime::currentSecsSinceEpoch());

        r.emplace_back(
            StandardItem::make(
                QStringLiteral("unix"), t, tr_unix, tr_unix, icon_urls,
                {
                    {
                        QStringLiteral("c"), tr_copy(),
                        [=](){ setClipboardText(t); }
                    }
                }
            ),
            (double)s.size() / tr_unix.size()
        );
    }

    if (utc.startsWith(query.string(), Qt::CaseInsensitive))
    {
        const QLocale loc;
        const QDateTime dt = QDateTime::currentDateTimeUtc();
        const auto sf = loc.toString(dt, QLocale::ShortFormat);
        const auto lf = loc.toString(dt, QLocale::LongFormat);

        r.emplace_back(
            StandardItem::make(
                utc, sf, tr("UTC date and time"), utc, icon_urls,
                {
                    {
                        QStringLiteral("scp"), tr_copy(),
                        [=](){ setClipboardText(sf); }
                    },
                    {
                        QStringLiteral("lcp"), tr_copy_with_placeholder().arg(lf),
                        [=](){ setClipboardText(lf); }
                    }
                }
            ),
            (double)s.size() / utc.size()
        );
    }

    bool isNumber;
    const ulong unixtime = s.toULong(&isNumber);
    if (isNumber)
    {
        const QLocale loc;
        const auto ls = loc.toString(QDateTime::fromSecsSinceEpoch(unixtime), QLocale::LongFormat);

        r.emplace_back(
            StandardItem::make(
                QStringLiteral("u2dt"), ls, tr("Date and time from unix time"), icon_urls,
                {
                    {
                        QStringLiteral("c"), tr_copy(),
                        [=](){ setClipboardText(ls); }
                    }
                }
            ),
            0.
        );
    }

    return r;
}

vector<shared_ptr<Item>> Plugin::handleEmptyQuery()
{
    if (show_date_on_empty_query_)
    {
        QLocale loc;
        QDateTime dt = QDateTime::currentDateTime();
        return {
            StandardItem::make(
                QStringLiteral("empty"),
                loc.toString(dt.time(), QLocale::ShortFormat),
                loc.toString(dt.date(), QLocale::LongFormat),
                icon_urls
                )
        };
    }
    return {};
}

QWidget *Plugin::buildConfigWidget()
{
    auto *w = new QWidget();
    Ui::ConfigWidget ui;
    ui.setupUi(w);

    ALBERT_PROPERTY_CONNECT_CHECKBOX(this, show_date_on_empty_query, ui.checkBox_emptyQuery)

    return w;
}
