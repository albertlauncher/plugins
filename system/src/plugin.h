// Copyright (c) 2017-2024 Manuel Schneider

#pragma once
#include "inhibitsleep.h"
#include <QStringList>
#include <albert/extensionplugin.h>
#include <albert/indexqueryhandler.h>
#include <albert/notification.h>
#include <albert/property.h>


enum SupportedCommands {
    LOCK,
    LOGOUT,
    SUSPEND,
#if not defined(Q_OS_MAC)
    HIBERNATE,
#endif
    REBOOT,
    POWEROFF
};

struct Command
{
    SupportedCommands id;
    const char * config_key_enabled;
    const char * config_key_title;
    const char * config_key_command;
    const QStringList icon_urls;
    const QString default_title;
    const QString description;
    const QString command;
};


class Plugin : public albert::ExtensionPlugin,
               public albert::IndexQueryHandler
{
    ALBERT_PLUGIN

public:

    Plugin();
    ~Plugin();

    void updateIndexItems() override;
    QWidget* buildConfigWidget() override;

    const std::vector<Command> commands;
    InhibitSleep inhibit_sleep;

    ALBERT_PLUGIN_PROPERTY_MEMBER(uint, default_timeout, inhibit_sleep.default_timeout, 60)

};
