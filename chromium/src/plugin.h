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

    QStringList defaultPaths() const;
    void resetPaths();
    void setPaths(const QStringList &paths);

    QFileSystemWatcher fs_watcher_;
    albert::BackgroundExecutor<std::vector<std::shared_ptr<BookmarkItem>>> indexer;
    QStringList paths_;
    bool index_hostname_;
    std::vector<std::shared_ptr<BookmarkItem>> bookmarks_;

signals:

    void statusChanged(const QString& status);

};
