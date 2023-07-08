// Copyright (c) 2022 Manuel Schneider

#pragma once
#include "albert/extension/queryhandler/triggerqueryhandler.h"
#include "albert/plugin.h"
#include <QClipboard>
#include <QDateTime>
#include <QTimer>

struct ClipboardEntry
{
    ClipboardEntry(QString t, QDateTime dt) : text(std::move(t)), datetime(dt) {}
    QString text;
    QDateTime datetime;
};


class Plugin : public albert::plugin::ExtensionPlugin<albert::TriggerQueryHandler>
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
    mutable bool persistent;
    uint length;
};
