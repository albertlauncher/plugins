// Copyright (c) 2023 Manuel Schneider

#include "embeddedmodule.h"  // Has to be first include
#include "plugin.h"
#include "config.h"
#include "pypluginloader.h"
#include <QDir>
#include <QEventLoop>
#include <QFileInfo>
#include <QFontDatabase>
#include <QMessageBox>
#include <QProcess>
#include <QRegularExpression>
#include <QStandardPaths>
#include <QTextEdit>
using namespace albert;
using namespace std;
namespace py = pybind11;

static const char *ATTR_PLUGIN_CLASS   = "Plugin";
static const char *ATTR_MD_IID         = "md_iid";
static const char *ATTR_MD_VERSION     = "md_version";
static const char *ATTR_MD_ID          = "md_id";
static const char *ATTR_MD_NAME        = "md_name";
static const char *ATTR_MD_DESCRIPTION = "md_description";
static const char *ATTR_MD_LICENSE     = "md_license";
static const char *ATTR_MD_URL         = "md_url";
static const char *ATTR_MD_MAINTAINERS = "md_maintainers";
static const char *ATTR_MD_BIN_DEPS    = "md_bin_dependencies";
static const char *ATTR_MD_LIB_DEPS    = "md_lib_dependencies";
static const char *ATTR_MD_CREDITS     = "md_credits";
static const char *ATTR_MD_PLATFORMS   = "md_platforms";
//static const char *ATTR_MD_MINPY     = "md_min_python";


PyPluginLoader::PyPluginLoader(Plugin &provider, const QFileInfo &file_info)
    : PluginLoader(file_info.absoluteFilePath()), provider_(provider)
{
    if(!file_info.exists())
        throw runtime_error("File path does not exist");
    else if (file_info.isFile()){
        if (path.endsWith(".py"))
            source_path_ = path;
        else
            throw NoPluginException();  // Path is not a python file
    }
    else if (QFileInfo fi(QDir(path).filePath("__init__.py")); fi.exists() && fi.isFile())
        source_path_ = fi.absoluteFilePath();
    else
        throw NoPluginException();  // Python package init file does not exist

    // Extract metadata

    metadata_.id = file_info.completeBaseName();

    QString source;

    if(QFile file(source_path_); file.open(QIODevice::ReadOnly))
        source = QTextStream(&file).readAll();
    else
        throw runtime_error(QString("Can't open source file: %1").arg(file.fileName()).toLatin1());

    //Parse the source code using ast and get all FunctionDef and Assign ast nodes
    py::gil_scoped_acquire acquire;
    py::module ast = py::module::import("ast");
    py::object ast_root = ast.attr("parse")(source.toStdString());

    map<QString, py::object> ast_assignments;

    for (auto node : ast_root.attr("body")){
        if (py::isinstance(node, ast.attr("Assign"))){
            auto py_value = node.attr("value");
            for (py::handle target : node.attr("targets")){
                if (py::isinstance(target, ast.attr("Name"))){
                    auto target_name = target.attr("id").cast<QString>();

                    if (py::isinstance(py_value, ast.attr("Str"))){
                        QString value = py_value.attr("value").cast<QString>();

                        if (target_name == ATTR_MD_IID)
                            metadata_.iid = value;

                        else if (target_name == ATTR_MD_ID)
                            metadata_.id = value;

                        else if (target_name == ATTR_MD_NAME)
                            metadata_.name = value;

                        else if (target_name == ATTR_MD_VERSION)
                            metadata_.version = value;

                        else if (target_name == ATTR_MD_DESCRIPTION)
                            metadata_.description = value;

                        else if (target_name == ATTR_MD_LICENSE)
                            metadata_.license = value;

                        else if (target_name == ATTR_MD_URL)
                            metadata_.url = value;

                        else if (target_name == ATTR_MD_MAINTAINERS)
                            metadata_.maintainers = {value};

                        else if (target_name == ATTR_MD_LIB_DEPS)
                            metadata_.runtime_dependencies = {value};

                        else if (target_name == ATTR_MD_BIN_DEPS)
                            metadata_.binary_dependencies = {value};

                        else if (target_name == ATTR_MD_CREDITS)
                            metadata_.third_party_credits = {value};
                    }

                    if (py::isinstance(py_value, ast.attr("List"))){
                        QStringList list;
                        for (const py::handle item : py_value.attr("elts").cast<py::list>())
                            if (py::isinstance(item, ast.attr("Str")))
                                list << item.attr("s").cast<py::str>().cast<QString>();

                        if (target_name == ATTR_MD_MAINTAINERS)
                            metadata_.maintainers = list;

                        else if (target_name == ATTR_MD_LIB_DEPS)
                            metadata_.runtime_dependencies = list;

                        else if (target_name == ATTR_MD_BIN_DEPS)
                            metadata_.binary_dependencies = list;

                        else if (target_name == ATTR_MD_CREDITS)
                            metadata_.third_party_credits = list;

                        else if (target_name == ATTR_MD_PLATFORMS)
                            metadata_.platforms = list;
                    }
                }
            }
        }
    }

    if (py::object obj = ast.attr("get_docstring")(ast_root); py::isinstance<py::str>(obj))
        metadata_.long_description = obj.cast<py::str>().cast<QString>();

    metadata_.load_type = LoadType::User;

    // Validate metadata

    if (metadata_.iid.isEmpty())
        throw NoPluginException();


    QStringList errors;
    static const QRegularExpression regex_version(R"R(^(\d+)\.(\d+)$)R");

    if (auto match = regex_version.match(metadata_.iid); !match.hasMatch())
        errors << QString("Invalid version format: '%1'. Expected <major>.<minor>.")
                      .arg(match.captured(0));
    else if (uint maj = match.captured(1).toUInt(); maj != MAJOR_INTERFACE_VERSION)
        errors << QString("Incompatible major interface version. Expected %1, got %2")
                      .arg(MAJOR_INTERFACE_VERSION).arg(maj);
    else if (uint min = match.captured(2).toUInt(); min > MINOR_INTERFACE_VERSION)
        errors << QString("Incompatible minor interface version. Up to %1 supported, got %2.")
                      .arg(MINOR_INTERFACE_VERSION).arg(min);

    if (!regex_version.match(metadata_.version).hasMatch())
        errors << "Invalid version scheme. Use '<version>.<patch>'.";

    static const QRegularExpression regex_id(R"R(\w+)R");
    if (!regex_id.match(metadata_.id).hasMatch())
        errors << QString("Invalid plugin id '%1'. Use [a-z0-9_].").arg(metadata_.id);

    if (metadata_.name.isEmpty())
        errors << "'name' must not be empty.";

    if (metadata_.description.isEmpty())
        errors << "'description' must not be empty.";

    if (!metadata_.platforms.isEmpty())
#if defined Q_OS_MACOS
        if (!metadata_.platforms.contains("Darwin"))
#elif defined(Q_OS_LINUX) || defined(Q_OS_FREEBSD)
        if (!metadata_.platforms.contains("Linux"))
#elif defined Q_OS_WIN
        if (!metadata_.platforms.contains("Windows"))
#endif
        errors << QString("Platform not supported. Supported: %1").arg(metadata_.platforms.join(", "));

    // QLoggingCategory does not take ownership of the cstr. Keep the std::string alive.
    logging_category_name = metadata_.id.toUtf8().toStdString();
    logging_category = make_unique<QLoggingCategory>(logging_category_name.c_str(), QtDebugMsg);

    // Finally set state based on errors
    if (!errors.isEmpty())
        throw runtime_error(errors.join(", ").toUtf8().constData());
}

PyPluginLoader::~PyPluginLoader() = default;

const QString &PyPluginLoader::source_path() const { return source_path_; }

const albert::PluginProvider &PyPluginLoader::provider() const { return provider_; }

const albert::PluginMetaData &PyPluginLoader::metaData() const { return metadata_; }

albert::PluginInstance *PyPluginLoader::instance() const { return cpp_plugin_instance_; }

QString PyPluginLoader::load_()
{
    py::gil_scoped_acquire acquire;

    // Import as __name__ = albert.package_name
    py::module importlib_util = py::module::import("importlib.util");
    py::object pyspec = importlib_util.attr("spec_from_file_location")(QString("albert.%1").arg(metadata_.id), source_path_); // Prefix to avoid conflicts
    module_ = importlib_util.attr("module_from_spec")(pyspec);

    // Set default md_id
    if (!py::hasattr(module_, ATTR_MD_ID))
        module_.attr("md_id") = metadata_.id;

    // Attach logcat functions
    // https://bugreports.qt.io/browse/QTBUG-117153
    // https://code.qt.io/cgit/pyside/pyside-setup.git/commit/?h=6.5&id=2823763072ce3a2da0210dbc014c6ad3195fbeff
    py::setattr(module_,"debug", py::cpp_function([this](const QString &s){ qCDebug((*logging_category),) << s; }));
    py::setattr(module_,"info", py::cpp_function([this](const QString &s){ qCInfo((*logging_category),) << s; }));
    py::setattr(module_,"warning", py::cpp_function([this](const QString &s){ qCWarning((*logging_category),) << s; }));
    py::setattr(module_,"critical", py::cpp_function([this](const QString &s){ qCCritical((*logging_category),) << s; }));

    // Execute module
    pyspec.attr("loader").attr("exec_module")(module_);

    // Instanciate plugin
    py_plugin_instance_ = module_.attr(ATTR_PLUGIN_CLASS)();  // may throw
    cpp_plugin_instance_ = py_plugin_instance_.cast<PluginInstance*>();  // may throw

    for (const auto& exec : metadata_.binary_dependencies)
        if (QStandardPaths::findExecutable(exec).isNull())
        throw runtime_error(QString("No '%1' in $PATH.").arg(exec).toStdString());

    return {};
}

QString PyPluginLoader::load()
{
    QString err;
    try
    {
        try {
            return load_();
        } catch (py::error_already_set &e) {

            // Catch only import errors, rethrow anything else
            if (e.matches(PyExc_ModuleNotFoundError)) {

                // ask user if dependencies should be installed
                QMessageBox mb;
                mb.setIcon(QMessageBox::Information);
                mb.setWindowTitle("Module not found");
                mb.setText(QString(
                    "Some modules in the plugin '%1' were not found. Probably "
                    "the plugin has mandatory dependencies which are not "
                    "installed on this system.\n\nInstall dependencies into "
                    "the Albert virtual environment?"
                ).arg(metadata_.name));
                mb.setStandardButtons(QMessageBox::Yes|QMessageBox::No);
                mb.setDefaultButton(QMessageBox::Yes);
                mb.setDetailedText(e.what());
                if (mb.exec() == QMessageBox::Yes){

                    // install dependencies

                    QProcess proc;
                    proc.start(
                        "python3",
                        {
                            "-m",
                            "pip",
                            "install",
                            "--disable-pip-version-check",
                            "--target",
                            provider_.dataDir()->filePath("site-packages"),
                            metadata_.runtime_dependencies.join(" ")
                        }
                    );

                    auto *te = new QTextEdit;
                    te->setCurrentFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));
                    te->setReadOnly(true);
                    te->setWindowTitle(QString("Installing '%1' dependencies").arg(metadata_.name));
                    te->resize(600, 480);
                    te->setAttribute(Qt::WA_DeleteOnClose);
                    te->show();

                    QObject::connect(&proc, &QProcess::readyReadStandardOutput, te,
                                     [te, &proc](){ te->append(QString::fromUtf8(proc.readAllStandardOutput())); });
                    QObject::connect(&proc, &QProcess::readyReadStandardError, te,
                                     [te, &proc](){ te->append(QString::fromUtf8(proc.readAllStandardError())); });

                    QEventLoop loop;
                    QObject::connect(&proc, &QProcess::finished, &loop, &QEventLoop::quit);
                    loop.exec();

                    if (proc.exitStatus() == QProcess::ExitStatus::NormalExit && proc.exitCode() == EXIT_SUCCESS){

                        te->deleteLater();  // auto-close on success only
                        return load_();  // On success try again

                    } else
                        err = "Installing dependencies failed.";
                } else
                    err = "Dependencies are missing.";
            } else
                throw e;
        }
    } catch(const std::exception &e) {
        err = e.what();
    } catch(...) {
        err = "Unknown exception while loading";
    }

    unload();
    return err;
}

QString PyPluginLoader::unload()
{
    try {
        py::gil_scoped_acquire acquire;
        cpp_plugin_instance_ = nullptr;
        py_plugin_instance_ = py::object();
        module_ = py::object();
        return {};
    } catch(std::exception const &e) {
        return QString("Error while unloading '%1': %2.").arg(metadata_.id, e.what());
    } catch(...) {
        return QString("Unknown error while unloading '%1'").arg(metadata_.id);
    }
}


