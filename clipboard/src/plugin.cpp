// Copyright (c) 2022 Manuel Schneider

#include "plugin.h"
#include <QGuiApplication>
using namespace std;
using namespace albert;

Plugin::Plugin() : clipboard(QGuiApplication::clipboard())
{
    connect(clipboard, &QClipboard::changed, this, [this](){
        auto text = clipboard->text();
        history.emplace_back(text, QDateTime::currentDateTime());
    });
}

void Plugin::handleTriggerQuery(TriggerQuery *query) const
{
    auto trimmed = query->string().trimmed();
    QLocale loc;
    int rank = 1;
    for (auto it = history.crbegin(); it != history.crend(); ++it, ++rank){
        const auto &entry = *it;
        auto display_text = entry.text;
        display_text.replace('\n', ' ');
        if (it->text.contains(trimmed))
            query->add(
                StandardItem::make(
                    id(),
                    display_text,
                    QString("#%1 %2").arg(rank).arg(loc.toString(entry.datetime, QLocale::LongFormat)),
                    {":clipboard"},
                    {
                        Action("copy", "Copy to clipboard", [t=entry.text](){ setClipboardText(t); })
                    }
                )
            );
    }
}


