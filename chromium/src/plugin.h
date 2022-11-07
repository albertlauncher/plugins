// Copyright (c) 2022 Manuel Schneider

#pragma once
#include "albert.h"
#include <QFileSystemWatcher>
#include <memory>

struct BookmarkItem;

class Plugin :
        public albert::ExtensionPlugin,
        public albert::IndexQueryHandler,
        public albert::ConfigWidgetProvider
{
    Q_OBJECT ALBERT_PLUGIN
public:
    Plugin();

    std::vector<albert::IndexItem> indexItems() const final;
    QWidget* buildConfigWidget() override;



private:
    QFileSystemWatcher file_system_watcher_;
    bool index_hostname_;
    QStringList paths_;
    std::vector<std::shared_ptr<BookmarkItem>> bookmarks_;
    albert::BackgroundExecutor<std::vector<std::shared_ptr<BookmarkItem>>> indexer;
    void setPaths(const QStringList &paths);
signals:
    void statusChanged(const QString& status);
};
