// Copyright (c) 2022 Manuel Schneider

#pragma once
#include <QObject>
#include <memory>
#include "albert.h"

class Plugin:
        public albert::ExtensionPlugin,
        public albert::IndexQueryHandler
{
    Q_OBJECT ALBERT_PLUGIN
public:
    Plugin();
    QString defaultTrigger() const override;
    QWidget* buildConfigWidget() override;
    void updateIndexItems() override;

public:
    QFileSystemWatcher fs_watcher;
    albert::BackgroundExecutor<std::vector<albert::IndexItem>> indexer;
};
