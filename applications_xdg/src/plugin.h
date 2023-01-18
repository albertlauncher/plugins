// Copyright (c) 2022 Manuel Schneider

#pragma once
#include "albert.h"
#include <memory>
#include <QFileSystemWatcher>


class Plugin:
        public albert::ExtensionPlugin,
        public albert::IndexQueryHandler,
        public albert::ConfigWidgetProvider

{
    Q_OBJECT ALBERT_PLUGIN
public:
    Plugin();

    void updateIndexItems() override;
    QWidget *buildConfigWidget() override;

protected:
    std::vector<albert::IndexItem> indexApps(const bool &abort) const;

    albert::BackgroundExecutor<std::vector<albert::IndexItem>> indexer;
    std::vector<albert::IndexItem> apps;
    QFileSystemWatcher fs_watcher_;
    bool ignoreShowInKeys;
    bool useKeywords;
    bool useGenericName;
    bool useNonLocalizedName;
};
