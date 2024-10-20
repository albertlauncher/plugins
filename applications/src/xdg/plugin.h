// Copyright (c) 2022-2024 Manuel Schneider

#pragma once
#include "pluginbase.h"
#include <QStringList>

class Plugin : public PluginBase
{
    ALBERT_PLUGIN

public:

    Plugin();
    ~Plugin();

    // albert::ExtensionPlugin
    QWidget *buildConfigWidget() override;

    using PluginBase::runTerminal;

    void runTerminal(QStringList commandline, const QString working_dir = {}) const;

    static const std::map<QString, QStringList> exec_args;

private:

    ALBERT_PLUGIN_PROPERTY(bool, ignore_show_in_keys, false)
    ALBERT_PLUGIN_PROPERTY(bool, use_exec, false)
    ALBERT_PLUGIN_PROPERTY(bool, use_generic_name, false)
    ALBERT_PLUGIN_PROPERTY(bool, use_keywords, false)

};
