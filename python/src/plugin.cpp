// Copyright (c) 2022-2024 Manuel Schneider

#include "cast_specialization.hpp"
#include "embeddedmodule.hpp"
// import pybind first

#include "plugin.h"
#include "pypluginloader.h"
#include "ui_configwidget.h"
#include <QDir>
#include <QFile>
#include <QFontDatabase>
#include <QMessageBox>
#include <QPointer>
#include <QProcess>
#include <QRegularExpression>
#include <QSettings>
#include <QStandardPaths>
#include <QTextEdit>
#include <QUrl>
#include <albert/albert.h>
#include <albert/extensionregistry.h>
#include <albert/logging.h>
#include <chrono>
ALBERT_LOGGING_CATEGORY("python")
using namespace albert;
using namespace std;
using namespace chrono;
using std::filesystem::path;
namespace py = pybind11;
#define XSTR(s) STR(s)
#define STR(s) #s

applications::Plugin *apps;  // used externally
static const char *BIN = "bin";
static const char *STUB_VERSION = "stub_version";
static const char *LIB = "lib";
static const char *PIP = "pip" XSTR(PY_MAJOR_VERSION) "." XSTR(PY_MINOR_VERSION);
static const char *PLUGINS = "plugins";
static const char *PYTHON = "python" XSTR(PY_MAJOR_VERSION) "." XSTR(PY_MINOR_VERSION);
static const char *SITE_PACKAGES = "site-packages";
static const char *STUB_FILE = "albert.pyi";
static const char *VENV = "venv";

static void dumpPyConfig(PyConfig &config)
{

    DEBG << "config.home" << QString::fromWCharArray(config.home);
    DEBG << "config.base_executable" << QString::fromWCharArray(config.base_executable);
    DEBG << "config.executable" << QString::fromWCharArray(config.executable);
    DEBG << "config.base_exec_prefix" << QString::fromWCharArray(config.base_exec_prefix);
    DEBG << "config.exec_prefix" << QString::fromWCharArray(config.exec_prefix);
    DEBG << "config.base_prefix" << QString::fromWCharArray(config.base_prefix);
    DEBG << "config.prefix" << QString::fromWCharArray(config.prefix);
    DEBG << "config.program_name" << QString::fromWCharArray(config.program_name);
    DEBG << "config.pythonpath_env" << QString::fromWCharArray(config.pythonpath_env);
    DEBG << "config.platlibdir" << QString::fromWCharArray(config.platlibdir);
    DEBG << "config.stdlib_dir" << QString::fromWCharArray(config.stdlib_dir);
    DEBG << "config.safe_path" << config.safe_path;
    DEBG << "config.install_signal_handlers" << config.install_signal_handlers;
    DEBG << "config.site_import" << config.site_import;
    DEBG << "config.user_site_directory" << config.user_site_directory;
    DEBG << "config.verbose" << config.verbose;
    DEBG << "config.module_search_paths_set" << config.module_search_paths_set;
    DEBG << "config.module_search_paths:";
    for (Py_ssize_t i = 0; i < config.module_search_paths.length; ++i)
        DEBG << " -" << QString::fromWCharArray(config.module_search_paths.items[i]);
}

// static void dumpSysAttributes(const py::module &sys)
// {
//     DEBG << "version          :" << sys.attr("version").cast<QString>();
//     DEBG << "executable       :" << sys.attr("executable").cast<QString>();
//     DEBG << "base_exec_prefix :" << sys.attr("base_exec_prefix").cast<QString>();
//     DEBG << "exec_prefix      :" << sys.attr("exec_prefix").cast<QString>();
//     DEBG << "base_prefix      :" << sys.attr("base_prefix").cast<QString>();
//     DEBG << "prefix           :" << sys.attr("prefix").cast<QString>();
//     DEBG << "path:";
//     for (const auto &path : sys.attr("path").cast<QStringList>())
//         DEBG << " -" << path;
// }

Plugin::Plugin()
{
    ::apps = apps.get();

    DEBG << "Python version:" << QString("%1.%2.%3")
                                     .arg(PY_MAJOR_VERSION)
                                     .arg(PY_MINOR_VERSION)
                                     .arg(PY_MICRO_VERSION);

    DEBG << "Pybind11 version:" << QString("%1.%2.%3")
                                       .arg(PYBIND11_VERSION_MAJOR)
                                       .arg(PYBIND11_VERSION_MINOR)
                                       .arg(PYBIND11_VERSION_PATCH);

    tryCreateDirectory(dataLocation() / PLUGINS);

    updateStubFile();

    initPythonInterpreter();

    // Add venv site packages to path
    py::module::import("site").attr("addsitedir")(siteDirPath().c_str());

    release_.reset(new py::gil_scoped_release);  // Gil is initially held.

    initVirtualEnvironment();

    plugins_ = scanPlugins();
}

Plugin::~Plugin()
{
    release_.reset();
    plugins_.clear();

    // Causes hard to debug crashes, mem leaked, but nobody will toggle it a lot
    // py::finalize_interpreter();
}

void Plugin::updateStubFile() const
{
    QFile stub_rc(QString(":%1").arg(STUB_FILE));
    QFile stub_fs(stubFilePath());
    auto interface_version = QString("%1.%2")
                                 .arg(PyPluginLoader::MAJOR_INTERFACE_VERSION)
                                 .arg(PyPluginLoader::MINOR_INTERFACE_VERSION);

    if (interface_version != state()->value(STUB_VERSION).toString()
        && stub_fs.exists() && !stub_fs.remove())
        WARN << "Failed removing former stub file" << stub_fs.error();

    if (!stub_fs.exists())
    {
        INFO << "Writing stub file to" << stub_fs.fileName();
        if (stub_rc.copy(stub_fs.fileName()))
            state()->setValue(STUB_VERSION, interface_version);
        else
            WARN << "Failed copying stub file to" << stub_fs.fileName() << stub_rc.error();
    }
}

void Plugin::initPythonInterpreter() const
{
    DEBG << "Initializing Python interpreter";
    PyConfig config;
    PyConfig_InitIsolatedConfig(&config);
    config.site_import = 0;
    dumpPyConfig(config);
    if (auto status = Py_InitializeFromConfig(&config); PyStatus_Exception(status))
        throw runtime_error(Plugin::tr("Failed initializing the interpreter: %1 %2")
                            .arg(status.func, status.err_msg).toStdString());
    PyConfig_Clear(&config);
    dumpPyConfig(config);
}

void Plugin::initVirtualEnvironment() const
{
    if (is_directory(venvPath()))
        return;

    py::gil_scoped_acquire acquire;

    auto system_python =
        path(py::module::import("sys").attr("prefix").cast<string>()) / BIN / PYTHON;

    // Create the venv
    QProcess p;
    p.start(system_python.c_str(), {"-m",
                                    "venv",
                                    //"--upgrade",
                                    //"--upgrade-deps",
                                    venvPath().c_str()});
    DEBG << "Initializing venv using system interpreter:"
         << (QStringList() << p.program() << p.arguments()).join(QChar::Space);
    p.waitForFinished(-1);
    if (auto out = p.readAllStandardOutput(); !out.isEmpty())
        DEBG << out;
    if (auto err = p.readAllStandardError(); !err.isEmpty())
        WARN << err;
    if (p.exitCode() != 0)
        throw runtime_error(tr("Failed initializing virtual environment. Exit code: %1.")
                                .arg(p.exitCode()).toStdString());
}

path Plugin::venvPath() const { return dataLocation() / VENV; }

path Plugin::siteDirPath() const { return venvPath() / LIB / PYTHON / SITE_PACKAGES; }

path Plugin::userPluginDirectoryPath() const { return dataLocation() / PLUGINS; }

path Plugin::stubFilePath() const { return userPluginDirectoryPath() / STUB_FILE; }

QStringList Plugin::pluginDirs() const
{
    using QSP = QStandardPaths;

    QStringList plugin_dirs;
    for (const auto &d : QSP::locateAll(QSP::AppDataLocation, id(), QSP::LocateDirectory))
        if (QDir data_dir{d}; data_dir.cd(PLUGINS))
            plugin_dirs << data_dir.path();

    return plugin_dirs;
}

vector<unique_ptr<PyPluginLoader>> Plugin::scanPlugins() const
{
    auto start = system_clock::now();

    vector<unique_ptr<PyPluginLoader>> plugins;
    for (const QString &plugin_dir : pluginDirs())
    {
        if (QDir dir{plugin_dir}; dir.exists())
        {
            DEBG << "Searching Python plugins in" << dir.absolutePath();
            for (const QFileInfo &file_info : dir.entryInfoList(QDir::Files
                                                                | QDir::Dirs
                                                                | QDir::NoDotAndDotDot))
            {
                try {
                    auto loader = make_unique<PyPluginLoader>(*this, file_info.absoluteFilePath());
                    DEBG << "Found valid Python plugin" << loader->path();
                    plugins.emplace_back(::move(loader));
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

    return plugins;
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

    ui.label_api_version->setText(QString("<a href=\"file://%1\">v%2.%3</a>")
                                  .arg(stubFilePath().c_str())
                                  .arg(PyPluginLoader::MAJOR_INTERFACE_VERSION)
                                  .arg(PyPluginLoader::MINOR_INTERFACE_VERSION));

    ui.label_python_version->setText(QString("%1.%2.%3")
                                     .arg(PY_MAJOR_VERSION)
                                     .arg(PY_MINOR_VERSION)
                                     .arg(PY_MICRO_VERSION));

    ui.label_pybind_version->setText(QString("%1.%2.%3")
                                     .arg(PYBIND11_VERSION_MAJOR)
                                     .arg(PYBIND11_VERSION_MINOR)
                                     .arg(PYBIND11_VERSION_PATCH));

    connect(ui.pushButton_venv_open, &QPushButton::clicked,
            this, [this]{ open(venvPath()); });

    connect(ui.pushButton_venv_reset, &QPushButton::clicked, this, [this]
    {
        auto text = tr("Resetting the virtual environment requires a restart.");

        using MB = QMessageBox;
        if (MB::question(nullptr, qApp->applicationDisplayName(), text, MB::Cancel | MB::Ok, MB::Ok)
            == QMessageBox::Ok)
        {
            QFile::moveToTrash(venvPath());
            restart();
        }
    });

    connect(ui.pushButton_userPluginDir, &QPushButton::clicked,
            this, [this]{ open(userPluginDirectoryPath()); });

    return w;
}

bool Plugin::installPackages(const QStringList &packages) const
{
    // Install dependencies
    QProcess p;
    p.setProgram((venvPath() / BIN / PIP).c_str());
    p.setArguments(QStringList{"install", "--disable-pip-version-check"} << packages);

    DEBG << QString("Installing %1. [%2]")
                .arg(packages.join(", "), (QStringList(p.program()) << p.arguments()).join(" "));

    p.start();

    QPointer<QTextEdit> te(new QTextEdit);
    te->setCurrentFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));
    te->setReadOnly(true);
    te->resize(600, 480);
    te->setAttribute(Qt::WA_DeleteOnClose);
    te->show();

    QObject::connect(&p, &QProcess::readyReadStandardOutput, te, [te, &p]
    {
        auto s = QString::fromUtf8(p.readAllStandardOutput());
        te->setTextColor(Qt::gray);
        te->append(s);
        for (const auto &l : s.split(QRegularExpression("[\r\n]"), Qt::SkipEmptyParts))
             DEBG << l;
    });

    bool have_std_err = false;
    QObject::connect(&p, &QProcess::readyReadStandardError, te, [&]
    {
        have_std_err = true;
        auto s = QString::fromUtf8(p.readAllStandardError());
        te->setTextColor(Qt::red);
        te->append(s);
        for (const auto &l : s.split(QRegularExpression("[\r\n]"), Qt::SkipEmptyParts))
             WARN << l;
    });

    QEventLoop loop;
    QObject::connect(&p, &QProcess::finished, &loop, &QEventLoop::quit);
    loop.exec();
    loop.processEvents();

    auto success = p.exitStatus() == QProcess::ExitStatus::NormalExit
                    && p.exitCode() == EXIT_SUCCESS;

    if (!have_std_err && success && te)
        te->close();  // auto-close on success only

    return success;
}
