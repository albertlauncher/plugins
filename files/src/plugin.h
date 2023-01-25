// Copyright (c) 2022 Manuel Schneider

#pragma once
#include "albert.h"
#include "fsindex.h"
#include "filebrowsers.h"

class Plugin:
        public albert::ExtensionPlugin,
        public albert::IndexQueryHandler
{
    Q_OBJECT ALBERT_PLUGIN

public:
    Plugin();
    ~Plugin();

    void updateIndexItems() override;
    QWidget *buildConfigWidget() override;
    FsIndex &fsIndex();

private:
    FsIndex fs_index_;
    std::shared_ptr<albert::Item> update_item;
    HomeBrowser homebrowser;
    RootBrowser rootbrowser;

signals:
    void statusInfo(const QString&) const;

};
