// Copyright (c) 2017-2024 Manuel Schneider

#pragma once
#include "albert/query/triggerqueryhandler.h"
#include "albert/util/backgroundexecutor.h"
#include "albert/util/extensionplugin.h"
#include <QFileSystemWatcher>
#include <set>

class Plugin : public albert::ExtensionPlugin,
               public albert::TriggerQueryHandler
{
    Q_OBJECT ALBERT_PLUGIN

public:
    Plugin();
    QString synopsis() const override { return "<command> [params]"; }
    QString defaultTrigger() const override { return ">"; }
    void handleTriggerQuery(albert::Query*) override;

private:
    QFileSystemWatcher watcher;
    std::set<QString> index;
    albert::BackgroundExecutor<std::set<QString>> indexer;
};
