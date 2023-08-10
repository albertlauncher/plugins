// Copyright (c) 2022-2023 Manuel Schneider

#include <pybind11/embed.h> // Has to be first import
#include "cast_specialization.h"
#include "albert/albert.h"
#include "albert/logging.h"
#include "albert/util/timeprinter.h"
#include "plugin.h"
#include "pypluginloader.h"
#include "ui_configwidget.h"
#include <QDir>
#include <QSettings>
#include <QStandardPaths>
ALBERT_LOGGING_CATEGORY("python")
using namespace albert;
using namespace std;
namespace py = pybind11;

static const constexpr char *PLUGIN_DIR = "plugins";
static const char *CFG_WATCH_SOURCES = "watchSources";
static const bool DEF_WATCH_SOURCES = false;


Plugin::Plugin()
{
    /*
     * The python interpreter is never unloaded once it has been loaded. This
     * is working around the ugly segfault that occur when third-party libraries
     * have been loaded an the interpreter is finalized and initialized.
     */
    if (!Py_IsInitialized())
        py::initialize_interpreter(false);
    release_.reset(new py::gil_scoped_release);

    py::gil_scoped_acquire acquire;

    py::module sys = py::module::import("sys");
    INFO << "Python version:" << sys.attr("version").cast<QString>();

    auto data_dir = dataDir();

    py::module::import("site").attr("addsitedir")(data_dir->filePath("site-packages"));

    // Create module dirs
    if (!data_dir->exists(PLUGIN_DIR))
        data_dir->mkdir(PLUGIN_DIR);

    TimePrinter tp("[%1 ms] Python plugin scan");
    using QSP = QStandardPaths;
    auto plugin_dirs = QSP::locateAll(QSP::AppDataLocation, id(), QSP::LocateDirectory);
    for (const QString &plugin_dir : plugin_dirs) {
        if (QDir dir{plugin_dir}; dir.cd(PLUGIN_DIR)) {
            DEBG << "Searching Python plugins in" << dir.absolutePath();
            for (const QFileInfo &file_info : dir.entryInfoList(QDir::Dirs|QDir::Files|QDir::NoDotAndDotDot)) {
                try {
                    auto &loader = plugins_.emplace_back(make_unique<PyPluginLoader>(*this, file_info));
                    DEBG << "Found valid Python plugin" << loader->path;
                } catch (const exception &e) {
                    WARN << e.what() << file_info.filePath();
                }
            }
        }
    }

    setWatchSources(settings()->value(CFG_WATCH_SOURCES, DEF_WATCH_SOURCES).toBool());
}

Plugin::~Plugin()
{
    release_.reset();
    plugins_.clear();
    py::finalize_interpreter();
}

bool Plugin::watchSources() const { return sources_watcher_.get(); }

void Plugin::setWatchSources(bool val)
{
    if (watchSources() && !val){
        sources_watcher_.reset();
    } else if (!watchSources() && val){
        sources_watcher_ = make_unique<QFileSystemWatcher>();
        connect(sources_watcher_.get(), &QFileSystemWatcher::fileChanged, this,  [this](const QString path){
            for (auto &loader : plugins_)
                if (path == loader->source_path())
                if (loader->state() == PluginState::Loaded ||
                    (loader->state() == PluginState::Unloaded && !loader->stateInfo().isEmpty())){
                    loader->unload();
                    loader->load();
                }
        });
//        if (!sources_watcher_->files().isEmpty())
//            sources_watcher_->removePaths(sources_watcher_->files());
        for (const auto &loader : plugins_)
            sources_watcher_->addPath(loader->source_path());
    }
    settings()->setValue(CFG_WATCH_SOURCES, val);
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

    ui.checkBox_watchSources->setChecked(watchSources());
    connect(ui.checkBox_watchSources, &QCheckBox::toggled,
            this, &Plugin::setWatchSources);

    connect(ui.pushButton_packages, &QPushButton::clicked, this, [this](){
        openUrl("file://" + dataDir()->filePath("site-packages"));
    });

    return w;
}

void Plugin::installPackages(const QStringList &package_names) const
{
    auto script = QString(R"R(python3 -m pip install --disable-pip-version-check --target "%1" %2; cd "%1")R")
                      .arg(dataDir()->filePath("site-packages"), package_names.join(" "));
    runTerminal(script);
}
