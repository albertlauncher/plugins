// Copyright (c) 2022-2024 Manuel Schneider

#pragma once
#include <albert/triggerqueryhandler.h>
#include <albert/extensionplugin.h>
#include <albert/plugindependency.h>
#include <QClipboard>
#include <QDateTime>
#include <QTimer>
#include <shared_mutex>
class Snippets;


struct ClipboardEntry
{
    // required to allow list<ClipboardEntry>::resize
    // actually never used.
    ClipboardEntry() = default;
    ClipboardEntry(QString t, QDateTime dt) : text(std::move(t)), datetime(dt) {}
    QString text;
    QDateTime datetime;
};


class Plugin : public albert::ExtensionPlugin,
               public albert::TriggerQueryHandler
{
    ALBERT_PLUGIN

public:

    Plugin();
    ~Plugin();

    QString defaultTrigger() const override;
    void handleTriggerQuery(albert::Query*) override;
    QWidget *buildConfigWidget() override;

private:
    void checkClipboard();

    QTimer timer;
    QClipboard * const clipboard;
    uint length;
    mutable std::list<ClipboardEntry> history;
    mutable bool persistent;
    mutable std::shared_mutex mutex;
    // explicit current, such that users can delete recent ones
    QString clipboard_text;
    
    albert::WeakDependency<Snippets> snippets;
};


