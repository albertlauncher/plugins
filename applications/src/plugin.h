// Copyright (c) 2022-2024 Manuel Schneider

#pragma once
#include "applications.h"
#include <QFileSystemWatcher>
#include <QStringList>
#include <albert/backgroundexecutor.h>
#include <albert/extensionplugin.h>
#include <albert/indexqueryhandler.h>
#include <albert/property.h>
#include <memory>

class Plugin : public albert::ExtensionPlugin,
               public albert::IndexQueryHandler,
               public applications::Applications
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

    // albert::ExtensionPlugin
    QWidget *buildConfigWidget() override;

    // albert::IndexQueryHandler
    QString defaultTrigger() const override;
    void updateIndexItems() override;

    // applications::Applications
    void runTerminal(const QString &script = {}, const QString &working_dir = {}, bool close_on_exit = false) const override;
    void runTerminal(const QStringList &commandline, const QString &working_dir = {}) const override;
    std::vector<albert::Action> actions(const QUrl&) const override;

private:

    static QStringList appDirectories();
#if defined(Q_OS_UNIX)
    static QString userShell();
#endif

    class Private;
    std::unique_ptr<Private> d;

    QFileSystemWatcher fs_watcher_;

};
