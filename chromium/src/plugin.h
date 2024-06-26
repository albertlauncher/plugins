// Copyright (c) 2022-2024 Manuel Schneider

#pragma once
#include <albert/indexqueryhandler.h>
#include <albert/backgroundexecutor.h>
#include <albert/extensionplugin.h>
#include <QFileSystemWatcher>
#include <memory>
class BookmarkItem;


class Plugin : public albert::ExtensionPlugin,
               public albert::IndexQueryHandler
{
    ALBERT_PLUGIN
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
