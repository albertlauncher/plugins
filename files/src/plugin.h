// Copyright (c) 2022 Manuel Schneider

#pragma once
#include "albert/extension/queryhandler/indexqueryhandler.h"
#include "albert/plugin.h"
#include "filebrowsers.h"
#include "fsindex.h"
#include <QSettings>

class Plugin : public albert::plugin::ExtensionPlugin,
               public albert::IndexQueryHandler
{
    Q_OBJECT ALBERT_PLUGIN
    ALBERT_PLUGIN_PROPERTY(bool, fs_browsers_case_sensitive, false)

public:
    Plugin();
    ~Plugin();

    void initialize(albert::ExtensionRegistry&, std::map<QString,PluginInstance*>) override;
    void finalize(albert::ExtensionRegistry&) override;
    QWidget *buildConfigWidget() override;

    void updateIndexItems() override;

    const FsIndex &fsIndex();
    void addPath(const QString&);
    void removePath(const QString&);

private:
    FsIndex fs_index_;
    std::shared_ptr<albert::Item> update_item;
    HomeBrowser homebrowser;
    RootBrowser rootbrowser;

signals:
    void statusInfo(const QString&);
};
