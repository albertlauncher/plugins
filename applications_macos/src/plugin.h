// Copyright (c) 2022 Manuel Schneider

#pragma once
#include "albert.h"
#include <memory>
#include <QFileSystemWatcher>

class Plugin:
    public albert::ExtensionPlugin,
    public albert::IndexQueryHandler
{
    Q_OBJECT ALBERT_PLUGIN
public:
    Plugin();

    void updateIndexItems() override;

protected:
    static std::vector<std::shared_ptr<albert::Item>> indexApps(const bool &abort);
    albert::BackgroundExecutor<std::vector<albert::IndexItem>> indexer;
    QFileSystemWatcher fs_watcher_;
};
