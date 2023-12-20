// Copyright (c) 2017-2024 Manuel Schneider

#pragma once
#include "albert/extension/pluginprovider/pluginprovider.h"
#include "albert/plugin.h"
#include <memory>
class PyPluginLoader;
namespace pybind11 { class gil_scoped_release; }

class Plugin : public albert::plugin::ExtensionPlugin,
               public albert::PluginProvider
{
    Q_OBJECT ALBERT_PLUGIN

public:

    Plugin();
    ~Plugin() override;

    void initialize(albert::ExtensionRegistry &registry,
                    std::map<QString,PluginInstance*> dependencies) override;
    void finalize(albert::ExtensionRegistry &registry) override;

    QWidget* buildConfigWidget() override;
    std::vector<albert::PluginLoader*> plugins() override;

private:

    std::vector<std::unique_ptr<PyPluginLoader>> plugins_;
    std::unique_ptr<pybind11::gil_scoped_release> release_;

};


