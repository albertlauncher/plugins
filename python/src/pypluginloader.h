// Copyright (c) 2017-2024 Manuel Schneider

#pragma once
#include "pybind11/pybind11.h"
#include "albert/extension/pluginprovider/pluginmetadata.h"
#include "albert/extension/pluginprovider/pluginloader.h"
#include <memory>
#include <QLoggingCategory>
namespace albert {
class PluginProvider;
}
class QFileInfo;
class Plugin;
class PyPluginInstanceTrampoline;

class NoPluginException: public std::exception
{
public:
    NoPluginException(const std::string &what): what_(what) {}
    const char *what() const noexcept override { return what_.c_str(); }
private:
    std::string what_;
};

class PyPluginLoader : public albert::PluginLoader
{
public:

    PyPluginLoader(Plugin &provider, const QString &module_path);
    ~PyPluginLoader();

    QString path() const override;
    const albert::PluginMetaData &metaData() const override;
    void load() override;
    void unload() override;
    albert::PluginInstance *createInstance() override;

private:

    void load_();

    const Plugin &provider_;

    const QString module_path_;
    QString source_path_;

    albert::PluginMetaData metadata_;
    std::string logging_category_name;
    std::unique_ptr<QLoggingCategory> logging_category;

    pybind11::module module_;
    pybind11::object instance_;

};





