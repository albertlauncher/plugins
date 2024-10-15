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
#include <vector>
class Terminal;
class QFormLayout;

class PluginBase : public albert::ExtensionPlugin,
                   public albert::IndexQueryHandler,
                   public applications::Plugin
{
    Q_OBJECT

public:

    void commonInitialize(std::unique_ptr<QSettings> &s);

    // albert::IndexQueryHandler
    QString defaultTrigger() const override;
    void updateIndexItems() override;

    // applications::Plugin
    void runTerminal(const QString &script) const override;

protected:

    void setUserTerminalFromConfig();
    QWidget *createTerminalFormWidget();
    void addBaseConfig(QFormLayout*);
    std::vector<albert::IndexItem> buildIndexItems() const;
    static QStringList camelCaseSplit(const QString &s);

    QFileSystemWatcher fs_watcher;
    albert::BackgroundExecutor<std::vector<std::shared_ptr<applications::Application>>> indexer;
    std::vector<std::shared_ptr<applications::Application>> applications;
    std::vector<Terminal*> terminals;
    Terminal* terminal = nullptr;

    ALBERT_PLUGIN_PROPERTY(bool, use_non_localized_name, false)
    ALBERT_PLUGIN_PROPERTY(bool, split_camel_case, true)
    ALBERT_PLUGIN_PROPERTY(bool, use_acronyms, true)

};
