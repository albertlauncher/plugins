// Copyright (c) 2022-2023 Manuel Schneider

#pragma once
#include "albert/extension/queryhandler/indexqueryhandler.h"
#include "albert/plugin.h"
#include "albert/util/backgroundexecutor.h"
#include <QFileSystemWatcher>
#include <QStringList>
#include <memory>
namespace albert { class StandardItem; }


class Plugin : public albert::plugin::ExtensionPlugin,
               public albert::IndexQueryHandler
{
    Q_OBJECT ALBERT_PLUGIN
public:
    Plugin();

    QString defaultTrigger() const override;
    void updateIndexItems() override;
    QWidget *buildConfigWidget() override;

protected:
    std::vector<albert::IndexItem> indexApps(const bool &abort) const;

    std::pair<std::shared_ptr<albert::StandardItem>, QStringList>
    parseDesktopEntry(const QString &id, const QString& path) const;

    albert::BackgroundExecutor<std::vector<albert::IndexItem>> indexer;
    std::vector<albert::IndexItem> apps;
    QFileSystemWatcher fs_watcher_;
    bool ignoreShowInKeys;
    bool useKeywords;
    bool useGenericName;
    bool useNonLocalizedName;
    bool useExec;
    const QStringList xdg_current_desktop;
};
