// Copyright (c) 2017-2022 Manuel Schneider

#pragma once
#include "albert.h"
#include <memory>
class PyPluginLoader;
namespace pybind11 { class gil_scoped_release; }

class Plugin:
    public albert::ExtensionPlugin,
    public albert::PluginProvider,
    public albert::ConfigWidgetProvider
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
    void installPackages(const QStringList&) const;

    std::vector<PyPluginLoader> plugins_;
    std::unique_ptr<pybind11::gil_scoped_release> release_;
    std::unique_ptr<QFileSystemWatcher> sources_watcher_;

    friend class PyPluginLoader;
};
