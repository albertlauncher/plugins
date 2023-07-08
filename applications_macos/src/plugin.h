// Copyright (c) 2022 Manuel Schneider

#pragma once
#include "albert/extension/queryhandler/indexqueryhandler.h"
#include "albert/plugin.h"
#include "albert/util/backgroundexecutor.h"
#include <QFileSystemWatcher>
#include <memory>

class Plugin : public albert::plugin::ExtensionPlugin<albert::IndexQueryHandler>
{
    Q_OBJECT ALBERT_PLUGIN
public:
    Plugin();

    QString defaultTrigger() const override;
    void updateIndexItems() override;

protected:
    static std::vector<std::shared_ptr<albert::Item>> indexApps(const bool &abort);
    albert::BackgroundExecutor<std::vector<albert::IndexItem>> indexer;
    QFileSystemWatcher fs_watcher_;
};
