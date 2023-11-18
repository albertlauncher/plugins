// Copyright (c) 2017-2023 Manuel Schneider

#pragma once
#include "albert/extension/pluginprovider/pluginprovider.h"
#include "albert/plugin.h"
#include <QFileSystemWatcher>
#include <memory>
class PyPluginLoader;
namespace pybind11 { class gil_scoped_release; }

class Plugin : public albert::plugin::ExtensionPlugin<albert::PluginProvider>
{
    Q_OBJECT ALBERT_PLUGIN
public:
    Plugin();
    ~Plugin() override;

    bool watchSources() const;
    void setWatchSources(bool);

protected:
    QWidget* buildConfigWidget() override;
    std::vector<albert::PluginLoader*> plugins() override;

private:
    void reloadModules();
    void updateSourceWatches();

    std::vector<std::unique_ptr<PyPluginLoader>> plugins_;
    std::unique_ptr<pybind11::gil_scoped_release> release_;
    std::unique_ptr<QFileSystemWatcher> sources_watcher_;
};


