// Copyright (c) 2023-2024 Manuel Schneider

#include "plugin.h"
#include <QDateTime>
#include <QLocale>
#include <QTimeZone>
#include <albert/matcher.h>
#include <albert/standarditem.h>
#include <albert/util.h>
using namespace albert::timezones;
using namespace albert;
using namespace std;

QString Plugin::defaultTrigger() const
{ return tr("tz "); }

void Plugin::handleTriggerQuery(Query *query)
{
    QLocale loc;
    auto utc = QDateTime::currentDateTimeUtc();
    const auto tr_copy = tr("Copy to clipboard");
    const auto tr_copy_placeholder = tr("Copy '%1' to clipboard");

    for (auto &tz_id_barray: QTimeZone::availableTimeZoneIds())
    {
        if (!query->isValid())
            return;

        auto tz = QTimeZone(tz_id_barray);
        auto dt = utc.toTimeZone(tz);

        auto tz_id = QString::fromLocal8Bit(tz_id_barray).replace("_", " ");
        auto tz_name_sf = tz.displayName(dt, QTimeZone::ShortName, loc);
        auto tz_name_lf = tz.displayName(dt, QTimeZone::LongName, loc);
        auto tz_name_of = tz.displayName(dt, QTimeZone::OffsetName, loc);

        Matcher matcher(query->string());

        if (matcher.match(tz_id) || matcher.match(tz_name_sf) || matcher.match(tz_name_lf))
        {
            QStringList tz_info{tz_id, tz_name_lf, tz_name_sf, tz_name_of};
            tz_info.removeDuplicates();

            auto sf = loc.toString(dt, QLocale::ShortFormat);
            auto lf = loc.toString(dt, QLocale::LongFormat);

            query->add(
                StandardItem::make(
                    tz_id, lf, tz_info.join(", "), tz_id, {QStringLiteral(":datetime")},
                    {
                        {
                            QStringLiteral("cl"), tr_copy,
                            [=]{ setClipboardText(lf); }
                        },
                        {
                            QStringLiteral("cl"), tr_copy_placeholder.arg(sf),
                            [=]{ setClipboardText(sf); }
                        }
                    }
                )
            );
        }
    }
}
