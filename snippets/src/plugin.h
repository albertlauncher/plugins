// Copyright (c) 2023 Manuel Schneider

#pragma once
#include <QFileSystemWatcher>
#include <QObject>
#include <memory>
#include "albert.h"
class QWidget;

class Plugin:
        public albert::ExtensionPlugin,
        public albert::IndexQueryHandler
{
    Q_OBJECT ALBERT_PLUGIN
public:
    Plugin();

    void newSnippet(QWidget *parent) const;
    void removeSnippet(const QString &file_name) const;

    const QString snippets_path;

private:
    QString defaultTrigger() const override;
    QWidget* buildConfigWidget() override;
    void updateIndexItems() override;
    QString synopsis() const override;
    void handleTriggerQuery(TriggerQuery *) const override;

    QFileSystemWatcher fs_watcher;
    albert::BackgroundExecutor<std::vector<albert::IndexItem>> indexer;
};
