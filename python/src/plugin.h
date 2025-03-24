// Copyright (c) 2017-2025 Manuel Schneider

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

    QWidget* buildConfigWidget() override;
    std::vector<albert::PluginLoader*> plugins() override;

    bool installPackages(const QStringList &packages) const;

private:

    void updateStubFile() const;
    void initPythonInterpreter() const;
    void initVirtualEnvironment() const;

    std::filesystem::path venvPath() const;
    std::filesystem::path siteDirPath() const;
    std::filesystem::path userPluginDirectoryPath() const;
    std::filesystem::path stubFilePath() const;

    std::vector<std::unique_ptr<PyPluginLoader>> scanPlugins() const;

    albert::StrongDependency<applications::Plugin> apps{"applications"};
    std::vector<std::unique_ptr<PyPluginLoader>> plugins_;
    std::unique_ptr<pybind11::gil_scoped_release> release_;

};


