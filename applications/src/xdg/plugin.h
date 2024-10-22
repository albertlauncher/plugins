// Copyright (c) 2022-2024 Manuel Schneider

#pragma once
#include "pluginbase.h"
#include <QStringList>
#include <albert/telemetryprovider.h>

class Plugin : public PluginBase,
               public albert::TelemetryProvider
{
    ALBERT_PLUGIN

public:

    Plugin();
    ~Plugin();

    // albert::ExtensionPlugin
    QWidget *buildConfigWidget() override;

    // albert::TelemetryProvider
    QJsonObject telemetryData() const override;

    using PluginBase::runTerminal;

    void runTerminal(QStringList commandline, const QString working_dir = {}) const;

    static const std::map<QString, QStringList> exec_args;

private:

    ALBERT_PLUGIN_PROPERTY(bool, ignore_show_in_keys, false)
    ALBERT_PLUGIN_PROPERTY(bool, use_exec, false)
    ALBERT_PLUGIN_PROPERTY(bool, use_generic_name, false)
    ALBERT_PLUGIN_PROPERTY(bool, use_keywords, false)

};
