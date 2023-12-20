// Copyright (c) 2022-2024 Manuel Schneider

#include <pybind11/embed.h> // Has to be first import
#include "albert/extensionregistry.h"
#include "cast_specialization.h"
#include "albert/albert.h"
#include "albert/logging.h"
#include "plugin.h"
#include "pypluginloader.h"
#include "ui_configwidget.h"
#include <QDir>
#include <QSettings>
#include <QStandardPaths>
#include <chrono>
ALBERT_LOGGING_CATEGORY("python")
using namespace albert;
using namespace std;
using namespace chrono;
namespace py = pybind11;

static const constexpr char *PLUGIN_DIR = "plugins";
static const constexpr char *SITE_PACKAGES= "site-packages";
// static const char *CFG_WATCH_SOURCES = "watchSources";
// static const bool DEF_WATCH_SOURCES = false;


Plugin::Plugin() = default;
Plugin::~Plugin() = default;

void Plugin::initialize(albert::ExtensionRegistry &registry, map<QString,PluginInstance*> dependencies)
{
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

        auto data_dir = dataDir();

        // Create and add site-packages dir
        if (!data_dir.exists(SITE_PACKAGES))
            if(!data_dir.mkdir(SITE_PACKAGES))
                throw tr("Failed creating site-packages dir %1").arg(data_dir.filePath(SITE_PACKAGES));
        py::module::import("site").attr("addsitedir")(data_dir.filePath(SITE_PACKAGES));

        // Create module dirs
        if (!data_dir.exists(PLUGIN_DIR))
            data_dir.mkdir(PLUGIN_DIR);

        auto start = system_clock::now();
        using QSP = QStandardPaths;
        auto plugin_dirs = QSP::locateAll(QSP::AppDataLocation, id(), QSP::LocateDirectory);
        for (const QString &plugin_dir : plugin_dirs) {
            if (QDir dir{plugin_dir}; dir.cd(PLUGIN_DIR)) {
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

    ExtensionPlugin::initialize(registry, dependencies);
}

void Plugin::finalize(albert::ExtensionRegistry &registry)
{
    // ExtensionPlugin::finalize(r);
    registry.deregisterExtension(this);
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

QWidget *Plugin::buildConfigWidget()
{
    auto *w = new QWidget;
    Ui::ConfigWidget ui;
    ui.setupUi(w);

    connect(ui.pushButton_packages, &QPushButton::clicked, this, [this](){
        openUrl("file://" + dataDir().filePath("site-packages"));
    });

    return w;
}
