// Copyright (c) 2022 Manuel Schneider

#pragma once
#include <albert/indexqueryhandler.h>
#include <albert/extensionplugin.h>

class Plugin : public albert::ExtensionPlugin,
               public albert::IndexQueryHandler
{
    ALBERT_PLUGIN

public:
    Plugin();
    ~Plugin();

    QString defaultTrigger() const override { return QStringLiteral("cn "); }
    void updateIndexItems() override;

private:
    class Private;
    std::unique_ptr<Private> d;

};
