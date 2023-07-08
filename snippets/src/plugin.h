// Copyright (c) 2023 Manuel Schneider

#pragma once
#include "albert/extension/queryhandler/indexqueryhandler.h"
#include "albert/plugin.h"
#include "albert/util/backgroundexecutor.h"
#include <QFileSystemWatcher>
#include <QObject>
#include <memory>
class QWidget;

class Plugin : public albert::plugin::ExtensionPlugin<albert::IndexQueryHandler>
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
    void handleTriggerQuery(TriggerQuery*) const override;

    QFileSystemWatcher fs_watcher;
    albert::BackgroundExecutor<std::vector<albert::IndexItem>> indexer;
};
