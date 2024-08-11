// Copyright (c) 2022-2024 Manuel Schneider

#include "cast_specialization.hpp"
#include "embeddedmodule.hpp"
// import pybind first

#include "plugin.h"
#include "pypluginloader.h"
#include "ui_configwidget.h"
#include <QDir>
#include <QSettings>
#include <QStandardPaths>
#include <albert/extensionregistry.h>
#include <albert/logging.h>
#include <albert/util.h>
#include <chrono>
ALBERT_LOGGING_CATEGORY("python")
using namespace albert;
using namespace std;
using namespace chrono;
namespace py = pybind11;

static const constexpr char *PLUGIN_DIR = "plugins";

applications::Plugin *apps;

Plugin::Plugin():
    apps(registry(), "applications")
{
    ::apps = apps.get();

    /*
     * The python interpreter is never unloaded once it has been loaded. This
     * is working around the ugly segfault that occur when third-party libraries
     * have been loaded an the interpreter is finalized and initialized.
     */
    if (!Py_IsInitialized())
        py::initialize_interpreter(false);
    release_.reset(new py::gil_scoped_release);

    {
        py::gil_scoped_acquire acquire;

        py::module sys = py::module::import("sys");
        DEBG << "Python version:" << sys.attr("version").cast<QString>();
        DEBG << "Python path:" << sys.attr("path").cast<QStringList>();

        auto data_dir = createOrThrow(dataLocation());

        // Create and add site-packages dir
        if(QDir dir(sitePackagesLocation()); !dir.mkpath("."))
            throw tr("Failed creating site-packages dir %1").arg(dir.path());
        else
            py::module::import("site").attr("addsitedir")(dir.path());

        // Create writeable plugin dir
        if(QDir dir(userPluginsLocation()); !dir.mkpath("."))
            throw tr("Failed creating writeable plugin dir %1").arg(data_dir.path());
        else
        {
            QFile src(":albert.pyi");
            QFile dst(dir.filePath("albert.pyi"));
            const char * k_stub_ver = "stub_version";
            auto v = QString("%1.%2")
                    .arg(PyPluginLoader::MAJOR_INTERFACE_VERSION)
                    .arg(PyPluginLoader::MINOR_INTERFACE_VERSION);

            if (v != state()->value(k_stub_ver).toString() && dst.exists() && !dst.remove())
                WARN << "Failed removing former interface spec" << dst.error();

            if (!dst.exists())
            {
                if (src.copy(dst.fileName())){
                    state()->setValue(k_stub_ver, v);
                    INFO << "Copied interface spec to" << dst.fileName();
                }
                else
                    WARN << "Failed copying interface spec to" << dst.fileName() << src.error();
            }
        }

        auto start = system_clock::now();
        for (const QString &plugin_dir : pluginsLocations()) {
            if (QDir dir{plugin_dir}; dir.exists()) {
                DEBG << "Searching Python plugins in" << dir.absolutePath();
                for (const QFileInfo &file_info : dir.entryInfoList(QDir::Files|QDir::Dirs|QDir::NoDotAndDotDot)) {
                    try {
                        auto &loader = plugins_.emplace_back(make_unique<PyPluginLoader>(*this, file_info.absoluteFilePath()));
                        DEBG << "Found valid Python plugin" << loader->path();
                    }
                    catch (const NoPluginException &e) {
                        DEBG << QString("Invalid plugin (%1): %2").arg(e.what(), file_info.filePath());
                    }
                    catch (const exception &e) {
                        WARN << e.what() << file_info.filePath();
                    }
                }
            }
        }
        INFO << QStringLiteral("[%1 ms] Python plugin scan")
                    .arg(duration_cast<milliseconds>(system_clock::now()-start).count());
    }
}

Plugin::~Plugin()
{
    release_.reset();
    plugins_.clear();

    // Causes hard to debug crashes, mem leaked, but nobody will toggle it a lot
    // py::finalize_interpreter();
}

vector<PluginLoader*> Plugin::plugins()
{
    vector<PluginLoader*> plugins;
    for (auto &plugin : plugins_)
        plugins.emplace_back(plugin.get());
    return plugins;
}

QString Plugin::userPluginsLocation() const
{
    return QDir(dataLocation()).filePath(PLUGIN_DIR);
}

QStringList Plugin::pluginsLocations() const
{
    QStringList pl;
    using SP = QStandardPaths;
    auto data_dirs = SP::locateAll(SP::AppDataLocation, id(), SP::LocateDirectory);
    for (const QString &data_dir : data_dirs)
        if (QDir dir{data_dir}; dir.cd(PLUGIN_DIR))
            pl << dir.path();
    return pl;
}

QString Plugin::sitePackagesLocation() const
{
    return QDir(dataLocation()).filePath("site-packages");
}

QString Plugin::stubLocation() const
{
    return QDir(userPluginsLocation()).filePath("albert.pyi");
}

QWidget *Plugin::buildConfigWidget()
{
    auto *w = new QWidget;
    Ui::ConfigWidget ui;
    ui.setupUi(w);

    connect(ui.pushButton_sitePackages, &QPushButton::clicked, this,
            [this](){ openUrl(QUrl::fromLocalFile(sitePackagesLocation())); });

    connect(ui.pushButton_stubFile, &QPushButton::clicked, this,
            [this](){ openUrl(QUrl::fromLocalFile(stubLocation())); });

    connect(ui.pushButton_userPluginDir, &QPushButton::clicked, this,
            [this](){ openUrl(QUrl::fromLocalFile(userPluginsLocation())); });

    return w;
}
