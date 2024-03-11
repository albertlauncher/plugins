// Copyright (c) 2017-2024 Manuel Schneider

#pragma once

#include <albert/extensionplugin.h>
#include <albert/pluginprovider.h>
#include <memory>
class PyPluginLoader;
namespace pybind11 { class gil_scoped_release; }

class Plugin : public albert::ExtensionPlugin,
               public albert::PluginProvider
{
    ALBERT_PLUGIN

public:

    Plugin();
    ~Plugin() override;

    QWidget* buildConfigWidget() override;
    std::vector<albert::PluginLoader*> plugins() override;

private:

    QStringList pluginsLocations() const;
    QString userPluginsLocation() const;
    QString sitePackagesLocation() const;
    QString stubLocation() const;

    std::vector<std::unique_ptr<PyPluginLoader>> plugins_;
    std::unique_ptr<pybind11::gil_scoped_release> release_;

};


