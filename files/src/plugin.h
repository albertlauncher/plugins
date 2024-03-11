// Copyright (c) 2022-2024 Manuel Schneider

#pragma once
#include "filebrowsers.h"
#include "fsindex.h"
#include <QObject>
#include <QSettings>
#include <albert/extensionplugin.h>
#include <albert/indexqueryhandler.h>
#include <albert/property.h>

class Plugin : public albert::ExtensionPlugin,
               public albert::IndexQueryHandler
{
    ALBERT_PLUGIN
    ALBERT_PLUGIN_PROPERTY(bool, fs_browsers_case_sensitive, false)

public:

    Plugin();
    ~Plugin();

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
