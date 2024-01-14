// Copyright (c) 2023-2024 Manuel Schneider

#pragma once
#include "albert/extension/queryhandler/indexqueryhandler.h"
#include "albert/plugin.h"
#include "albert/util/backgroundexecutor.h"
#include "snippets.h"
#include <QFileSystemWatcher>
#include <QObject>
class QWidget;

class Plugin : public albert::plugin::ExtensionPlugin,
               public albert::IndexQueryHandler,
               public Snippets
{
    Q_OBJECT ALBERT_PLUGIN
public:
    Plugin();

    void addSnippet(const QString &text = {}, QWidget *modal_parent = nullptr) const override;
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
