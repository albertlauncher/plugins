// Copyright (c) 2022 Manuel Schneider

#pragma once
#include "albert.h"
#include <QFileSystemWatcher>
#include "albert/util/backgroundexecutor.h"
#include <QObject>
#include <memory>
#include <vector>

struct SshItem;

class Plugin:
        public albert::ExtensionPlugin,
        public albert::IndexQueryHandler,
        public albert::ConfigWidgetProvider
{
    Q_OBJECT ALBERT_PLUGIN
public:
    Plugin();
    QString synopsis() const override;
    void handleQuery(Query&) const override;
    std::vector<albert::IndexItem> indexItems() const override;
    QWidget* buildConfigWidget() override;

private:
    bool useKnownHosts() const;
    void setUseKnownHosts(bool b = true);

    albert::BackgroundExecutor<std::vector<std::shared_ptr<SshItem>>> indexer;
    QFileSystemWatcher fs_watcher_;
    std::vector<std::shared_ptr<SshItem>> hosts_;
    bool useKnownHosts_;
};
