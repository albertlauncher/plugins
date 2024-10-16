// Copyright (c) 2022-2024 Manuel Schneider

#include "cast_specialization.hpp"
#include "embeddedmodule.hpp"
// import pybind first

#include "plugin.h"
#include "pypluginloader.h"
#include "ui_configwidget.h"
#include <QDir>
#include <QFontDatabase>
#include <QPointer>
#include <QProcess>
#include <QRegularExpression>
#include <QSettings>
#include <QStandardPaths>
#include <QTextEdit>
#include <QUrl>
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
    if (Py_IsInitialized() != 0)
        throw runtime_error("The interpreter is already running");

    ::apps = apps.get();
    auto data_dir = createOrThrow(dataLocation());


    // Create writeable plugin dir containing the interface spec
    if(QDir dir(userPluginsLocation()); dir.mkpath("."))
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
    else
        throw tr("Failed creating writeable plugin dir %1").arg(data_dir.path());


    // Initialize the Python interpreter
    INFO << "Python version" << QString("%1.%2.%3")
            .arg(PY_MAJOR_VERSION).arg(PY_MINOR_VERSION).arg(PY_MICRO_VERSION);
    PyConfig config;
    // DEBG << "Py_GetExecPrefix()      " << QString::fromWCharArray(Py_GetExecPrefix());
    // DEBG << "Py_GetPath()            " << QString::fromWCharArray(Py_GetPath());
    // DEBG << "Py_GetPrefix()          " << QString::fromWCharArray(Py_GetPrefix());
    // DEBG << "Py_GetProgramFullPath() " << QString::fromWCharArray(Py_GetProgramFullPath());
    // DEBG << "Py_GetPythonHome()      " << QString::fromWCharArray(Py_GetPythonHome());
    // DEBG << "Py_GetVersion():        " << QString::fromLatin1(Py_GetVersion());
    PyConfig_InitIsolatedConfig(&config);
    // DEBG << "config.home" << QString::fromWCharArray(config.home);
    // DEBG << "config.base_executable" << QString::fromWCharArray(config.base_executable);
    // DEBG << "config.executable" << QString::fromWCharArray(config.executable);
    // DEBG << "config.base_exec_prefix" << QString::fromWCharArray(config.base_exec_prefix);
    // DEBG << "config.exec_prefix" << QString::fromWCharArray(config.exec_prefix);
    // DEBG << "config.base_prefix" << QString::fromWCharArray(config.base_prefix);
    // DEBG << "config.prefix" << QString::fromWCharArray(config.prefix);
    // DEBG << "config.program_name" << QString::fromWCharArray(config.program_name);
    // DEBG << "config.pythonpath_env" << QString::fromWCharArray(config.pythonpath_env);
    // DEBG << "config.platlibdir" << QString::fromWCharArray(config.platlibdir);
    // DEBG << "config.stdlib_dir" << QString::fromWCharArray(config.stdlib_dir);
    // DEBG << "config.safe_path" << config.safe_path;
    // DEBG << "config.install_signal_handlers" << config.install_signal_handlers;
    // DEBG << "config.site_import" << config.site_import;
    // DEBG << "config.user_site_directory" << config.user_site_directory;
    // DEBG << "config.verbose" << config.verbose;

    config.site_import = 0;

    DEBG << "Initializing Python interpreter";
    if (auto status = Py_InitializeFromConfig(&config); PyStatus_Exception(status))
        throw runtime_error(tr("Failed initializing the interpreter: %1 %2")
                            .arg(status.func, status.err_msg).toStdString());
    else{
        DEBG << "Successfully initialized python interpreter";
        // Gil is initially held. Release it forever.
        release_.reset(new py::gil_scoped_release);
    }

    PyConfig_Clear(&config);
    py::gil_scoped_acquire acquire;
    auto sys = py::module::import("sys");


    // Initialize the virtual environment using the system interpreter
    QProcess p;
    p.start(QDir(sys.attr("prefix").cast<QString>()).filePath("bin/python3"),
            {"-m", "venv", "--upgrade", "--upgrade-deps", venv()});
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


    // Add venv site packages to path
    py::module::import("site").attr("addsitedir")(sitePackagesLocation());


    // Set sys.executable to the venv python
    sys.attr("executable") = venv_python();


    // Debug
    DEBG << "version          :" << sys.attr("version").cast<QString>();
    DEBG << "executable       :" << sys.attr("executable").cast<QString>();
    DEBG << "base_exec_prefix :" << sys.attr("base_exec_prefix").cast<QString>();
    DEBG << "exec_prefix      :" << sys.attr("exec_prefix").cast<QString>();
    DEBG << "base_prefix      :" << sys.attr("base_prefix").cast<QString>();
    DEBG << "prefix           :" << sys.attr("prefix").cast<QString>();
    for (const auto &path : sys.attr("path").cast<QStringList>())
        DEBG << "path:            :" << path;


    // Find plugins

    QStringList plugin_dirs;
    using SP = QStandardPaths;
    auto data_dirs = SP::locateAll(SP::AppDataLocation, id(), SP::LocateDirectory);
    for (const auto &dd : data_dirs)
        if (QDir dir{dd}; dir.cd(PLUGIN_DIR))
            plugin_dirs << dir.path();

    auto start = system_clock::now();
    for (const QString &plugin_dir : plugin_dirs)
    {
        if (QDir dir{plugin_dir}; dir.exists())
        {
            DEBG << "Searching Python plugins in" << dir.absolutePath();
            for (const QFileInfo &file_info : dir.entryInfoList(QDir::Files|QDir::Dirs|QDir::NoDotAndDotDot))
            {
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


QString Plugin::venv() const
{ return QDir(dataLocation()).filePath("venv"); }

QString Plugin::venv_python() const
{ return QDir(venv()).filePath("bin/python3"); }

QString Plugin::venv_pip() const
{ return QDir(venv()).filePath("bin/pip3"); }

QString Plugin::sitePackagesLocation() const
{
    return QDir(venv()).filePath(
        QString("lib/python%2.%3/site-packages").arg(PY_MAJOR_VERSION).arg(PY_MINOR_VERSION)
    );
}

QString Plugin::userPluginsLocation() const
{ return QDir(dataLocation()).filePath(PLUGIN_DIR); }

QString Plugin::stubLocation() const
{ return QDir(userPluginsLocation()).filePath("albert.pyi"); }

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

bool Plugin::installPackages(const QStringList &packages)
{
    // Install dependencies
    QProcess p;
    p.start(venv_pip(), QStringList{"install"} << packages);
    DEBG << QString("Installing %1. [%2]")
            .arg(packages.join(", "), (QStringList(p.program()) << p.arguments()).join(" "));

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
