// Copyright (c) 2022-2024 Manuel Schneider

#pragma once
#include "albert/extension/queryhandler/triggerqueryhandler.h"
#include "albert/plugin.h"
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



class Plugin : public albert::plugin::ExtensionPlugin,
               public albert::TriggerQueryHandler
{
    Q_OBJECT ALBERT_PLUGIN
public:
    Plugin();
    ~Plugin();

    void initialize(albert::ExtensionRegistry&, std::map<QString,PluginInstance*>) override;

    QString defaultTrigger() const override;
    void handleTriggerQuery(TriggerQuery*) const override;
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
    
    Snippets *snippets;
};


