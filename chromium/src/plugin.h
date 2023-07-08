// Copyright (c) 2022-2023 Manuel Schneider

#pragma once
#include "albert/extension/queryhandler/indexqueryhandler.h"
#include "albert/plugin.h"
#include "albert/util/backgroundexecutor.h"
#include <QFileSystemWatcher>
#include <memory>

struct BookmarkItem;

class Plugin : public albert::plugin::ExtensionPlugin<albert::IndexQueryHandler>
{
    Q_OBJECT ALBERT_PLUGIN
public:
    Plugin();
    void updateIndexItems() override;
    QWidget* buildConfigWidget() override;

private:
    QFileSystemWatcher file_system_watcher_;
    bool index_hostname_;
    QStringList paths_;
    std::vector<std::shared_ptr<BookmarkItem>> bookmarks_;
    albert::BackgroundExecutor<std::vector<std::shared_ptr<BookmarkItem>>> indexer;
    void setPaths(const QStringList &paths);
    void resetPaths();
signals:
    void statusChanged(const QString& status);
};
