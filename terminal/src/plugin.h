// Copyright (c) 2022 Manuel Schneider

#pragma once
#include "albert.h"
#include <set>

class Plugin:
        public albert::ExtensionPlugin,
        public albert::TriggerQueryHandler
{
    Q_OBJECT ALBERT_PLUGIN

public:
    Plugin();
    QString synopsis() const override { return "<command> [params]"; }
    QString defaultTrigger() const override { return ">"; }
    void handleTriggerQuery(TriggerQuery &query) const override;

private:
    QFileSystemWatcher watcher;
    std::set<QString> index;
    albert::BackgroundExecutor<std::set<QString>> indexer;
};
