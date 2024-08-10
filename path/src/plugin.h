// Copyright (c) 2017-2024 Manuel Schneider

#pragma once
#include <QFileSystemWatcher>
#include <albert/backgroundexecutor.h>
#include <albert/extensionplugin.h>
#include <albert/plugin/applications.h>
#include <albert/plugindependency.h>
#include <albert/triggerqueryhandler.h>
#include <set>
namespace albert { class Action; }

class Plugin : public albert::ExtensionPlugin,
               public albert::TriggerQueryHandler
{
    ALBERT_PLUGIN

public:

    Plugin();
    ~Plugin();
    QString synopsis() const override { return "<command> [params]"; }
    QString defaultTrigger() const override { return ">"; }
    void handleTriggerQuery(albert::Query*) override;

private:

    std::vector<albert::Action> buildActions(const QString &commandline);

    QFileSystemWatcher watcher_;
    std::set<QString> index_;
    albert::BackgroundExecutor<std::set<QString>> indexer_;
    albert::StrongDependency<applications::Plugin> apps_;

};
