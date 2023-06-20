// Copyright (c) 2022 Manuel Schneider

#pragma once
#include "albert.h"
#include <QClipboard>

struct ClipboardEntry
{
    ClipboardEntry(QString text, QDateTime datetime) : text(std::move(text)), datetime(datetime) {}
    QString text;
    QDateTime datetime;
};


class Plugin : public albert::ExtensionPlugin,
               public albert::TriggerQueryHandler
{
    Q_OBJECT ALBERT_PLUGIN
public:
    Plugin();
    QString defaultTrigger() const override { return "cb "; }
    void handleTriggerQuery(TriggerQuery*) const override;
    QClipboard * const clipboard;
    std::vector<ClipboardEntry> history;
};
