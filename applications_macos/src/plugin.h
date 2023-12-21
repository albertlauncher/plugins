// Copyright (c) 2022 Manuel Schneider

#pragma once
#include "albert/extension/queryhandler/indexqueryhandler.h"
#include "albert/plugin.h"
#include <memory>

class Plugin : public albert::plugin::ExtensionPlugin,
               public albert::IndexQueryHandler
{
    Q_OBJECT ALBERT_PLUGIN
public:
    Plugin();
    ~Plugin();

    QString defaultTrigger() const override;
    void updateIndexItems() override;

protected:
    class Private;
    std::unique_ptr<Private> d;
};
