// Copyright (c) 2022 Manuel Schneider

#pragma once
#include "albert.h"
#include <QClipboard>

struct ClipboardEntry
{
    ClipboardEntry(QString t, QDateTime dt) : text(std::move(t)), datetime(dt) {}
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
    QWidget *buildConfigWidget() override;

    void writeHistory() const;
    void readHistory();

    QTimer timer;
    QClipboard * const clipboard;
    mutable std::list<ClipboardEntry> history;
    bool persistent;
    uint length;
};
