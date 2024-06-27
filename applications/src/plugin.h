// Copyright (c) 2022-2024 Manuel Schneider

#pragma once
#include <QFileSystemWatcher>
#include <QStringList>
#include <albert/backgroundexecutor.h>
#include <albert/extensionplugin.h>
#include <albert/indexqueryhandler.h>
#include <albert/property.h>
#include <memory>

class Plugin : public albert::ExtensionPlugin,
               public albert::IndexQueryHandler
{
    ALBERT_PLUGIN
    ALBERT_PLUGIN_PROPERTY(bool, use_non_localized_name, false)
#if defined(Q_OS_MACOS)
#elif defined(Q_OS_UNIX)
    ALBERT_PLUGIN_PROPERTY(bool, ignore_show_in_keys, false)
    ALBERT_PLUGIN_PROPERTY(bool, use_exec, false)
    ALBERT_PLUGIN_PROPERTY(bool, use_generic_name, false)
    ALBERT_PLUGIN_PROPERTY(bool, use_keywords, false)
#endif

public:

    Plugin();
    ~Plugin();

    void commonInitialize(std::unique_ptr<QSettings> &s);

    QString defaultTrigger() const override;
    void updateIndexItems() override;
    QWidget *buildConfigWidget() override;

private:

    static QStringList appDirectories();

    class Private;
    std::unique_ptr<Private> d;

    albert::BackgroundExecutor<std::vector<albert::IndexItem>> indexer_;
    QFileSystemWatcher fs_watcher_;

};
