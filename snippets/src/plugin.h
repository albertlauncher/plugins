// Copyright (c) 2023-2024 Manuel Schneider

#pragma once

#include "snippets.h"
#include <QFileSystemWatcher>
#include <albert/backgroundexecutor.h>
#include <albert/extensionplugin.h>
#include <albert/indexqueryhandler.h>
class QWidget;

class Plugin : public albert::ExtensionPlugin,
               public albert::IndexQueryHandler,
               public Snippets
{
    ALBERT_PLUGIN
public:
    Plugin();

    void addSnippet(const QString &text = {}, QWidget *modal_parent = nullptr) const override;
    void removeSnippet(const QString &file_name) const;

private:
    QString defaultTrigger() const override;
    QWidget* buildConfigWidget() override;
    void updateIndexItems() override;
    QString synopsis() const override;
    void handleTriggerQuery(albert::Query*) override;

    QFileSystemWatcher fs_watcher;
    albert::BackgroundExecutor<std::vector<albert::IndexItem>> indexer;
};
