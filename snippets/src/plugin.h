// Copyright (c) 2022 Manuel Schneider

#pragma once
#include <QObject>
#include <memory>
#include "albert.h"

class Plugin:
        public albert::ExtensionPlugin,
        public albert::IndexQueryHandler,
        public albert::ConfigWidgetProvider
{
    Q_OBJECT ALBERT_PLUGIN
public:
    Plugin();
    QString defaultTrigger() const override;
    QWidget* buildConfigWidget() override;
    std::vector<albert::IndexItem> indexItems() const override;

public:
    QFileSystemWatcher fs_watcher;
};
