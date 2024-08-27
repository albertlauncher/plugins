// Copyright (c) 2017-2024 Manuel Schneider

#pragma once
#include "pybind11/gil.h"

#include <albert/extensionplugin.h>
#include <albert/plugin/applications.h>
#include <albert/plugindependency.h>
#include <albert/pluginprovider.h>
#include <memory>
class PyPluginLoader;

class Plugin : public albert::ExtensionPlugin,
               public albert::PluginProvider
{
    ALBERT_PLUGIN

public:

    Plugin();
    ~Plugin() override;
    bool installPackages(const QStringList &packages);

    QWidget* buildConfigWidget() override;
    std::vector<albert::PluginLoader*> plugins() override;

private:

    inline QString venv() const;
    inline QString venv_pip() const;
    inline QString venv_python() const;
    inline QString sitePackagesLocation() const;
    inline QString userPluginsLocation() const;
    inline QString stubLocation() const;

    albert::StrongDependency<applications::Plugin> apps;
    std::vector<std::unique_ptr<PyPluginLoader>> plugins_;
    std::unique_ptr<pybind11::gil_scoped_release> release_;

};


