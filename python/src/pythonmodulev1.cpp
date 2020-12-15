// Copyright (c) 2017-2018 Manuel Schneider

// CAUTION ORDER MATTERS!
#include <pybind11/embed.h>
#include <pybind11/stl.h>
#include "pythonmodulev1.h"
#include <QByteArray>
#include <QDebug>
#include <QDirIterator>
#include <QFileInfo>
#include <QFileSystemWatcher>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLabel>
#include <QProcess>
#include <QProcess>
#include <QRegularExpression>
#include <QStandardPaths>
#include <QVBoxLayout>
#include <functional>
#include <vector>
#include "cast_specialization.h"
#include "albert/query.h"
#include "albert/util/standarditem.h"
#include "xdg/iconlookup.h"
Q_LOGGING_CATEGORY(qlc_python_modulev1, "python.modulev1")
#define DEBG qCDebug(qlc_python_modulev1,).noquote()
#define INFO qCInfo(qlc_python_modulev1,).noquote()
#define WARN qCWarning(qlc_python_modulev1,).noquote()
#define CRIT qCCritical(qlc_python_modulev1,).noquote()
using namespace std;
using namespace Core;
namespace py = pybind11;


namespace {
uint majorInterfaceVersion = 0;
uint minorInterfaceVersion = 4;

enum Target {
    VERSION,
    TITLE,
    AUTHORS,
    EXEC_DEPS,
    PY_DEPS,
    TRIGGERS
};
const QStringList targetNames = {
    "__version__",
    "__title__",
    "__authors__",
    "__exec_deps__",
    "__py_deps__",
    "__triggers__"
};
}


class Python::PythonModuleV1Private
{
public:
    QString path;
    QString sourceFilePath;
    QString id;  // id, __name__, Effectively the module name

    PythonModuleV1::State state;
    QString errorString;

    py::module module;

    struct Spec {
        QString name;
        QString description;
        QString version;
        QStringList authors;
        QStringList executableDependecies;
        QStringList pythonDependecies;
        QStringList triggers;
    } spec;
};

/** ***************************************************************************/
Python::PythonModuleV1::PythonModuleV1(const QString &path) : d(new PythonModuleV1Private) {

    d->path = path;
    QFileInfo fileInfo{d->path};

    if (!fileInfo.exists())
        throw std::runtime_error("Path does not exist");
    else if (fileInfo.isDir()){
        QDir dir{path};
        if (dir.exists("__init__.py"))
            d->sourceFilePath = dir.filePath("__init__.py");
        else
            throw std::runtime_error("Dir does not contain an init file");
    }
    else if (fileInfo.isFile())
        d->sourceFilePath = fileInfo.absoluteFilePath();
    else
        qFatal("This should never happen");

    d->spec.name = d->id = fileInfo.completeBaseName();
    d->state = State::InvalidMetadata;

    readMetadata();
}

void Python::PythonModuleV1::readMetadata() {

    DEBG << "Reading metadata of python module:" << QFileInfo(d->path).fileName();

    py::gil_scoped_acquire acquire;

    try {

        /*
         * Parse the source code using ast
         */

        // Get the extension spec source code

        QFile file(d->sourceFilePath);
        if(!file.open(QIODevice::ReadOnly))
            throw QString("Cant open init file: %1").arg(d->sourceFilePath);
        QString source = QTextStream(&file).readAll();
        file.close();

        // Parse it with ast and get all FunctionDef and Assign ast nodes

        py::module ast = py::module::import("ast");
        py::object ast_root = ast.attr("parse")(source.toStdString());

        std::map<QString, py::object> metadata_values;
        for (auto node : ast_root.attr("body")){

            if (py::isinstance(node, ast.attr("FunctionDef")))
                metadata_values.emplace(node.attr("name").cast<QString>(), node.attr("args").attr("args"));

            if (py::isinstance(node, ast.attr("Assign"))){
                py::list targets = node.attr("targets");
                if (py::len(targets) == 1 && py::isinstance(targets[0], ast.attr("Name"))){
                    QString targetName = targets[0].attr("id").cast<QString>();
                    if (targetNames.contains(targetName))
                        metadata_values.emplace(targetName, node.attr("value"));
                }
            }
        }

        /*
         * Check/get the mandatory metadata and check/get functions
         */

        py::object obj;

        // Check VERSION

        {
            auto targetName = targetNames[Target::VERSION];
            if (!metadata_values.count(targetName))
                throw QString("Module has no %1 specified").arg(targetName);

            if (!py::isinstance(obj = metadata_values[targetName], ast.attr("Str")))
                throw QString("%1 is not of type ast.Str").arg(targetName);

            d->spec.version = obj.attr("s").cast<py::str>().cast<QString>();

            QRegularExpression re("^(\\d)\\.(\\d)\\.(\\d)$");
            QRegularExpressionMatch match = re.match(d->spec.version);
            if (!match.hasMatch())
                throw QString("Invalid version format: '%1'. Expected '%2'.").arg(match.captured(0), re.pattern());

            uint maj = match.captured(1).toUInt();
            if (maj != majorInterfaceVersion)
                throw QString("Incompatible major interface version. Expected %1, got %2").arg(majorInterfaceVersion).arg(maj);

            uint min = match.captured(2).toUInt();
            if (min > minorInterfaceVersion)
                throw QString("Incompatible minor interface version. Up to %1 supported, got %2. Maybe you should update albert.").arg(minorInterfaceVersion).arg(min);
        }

        // Get pretty NAME

        {
            auto targetName = targetNames[Target::TITLE];
            if (!metadata_values.count(targetName))
                throw QString("Module has no %1 specified").arg(targetName);

            if (!py::isinstance(obj = metadata_values[targetName], ast.attr("Str")))
                throw QString("%1 is not of type ast.Str").arg(targetName);

            d->spec.name = obj.attr("s").cast<py::str>().cast<QString>();

        }

        // Get description/docstring

        {
            obj = ast.attr("get_docstring")(ast_root);
            if (py::isinstance<py::str>(obj))
                d->spec.description = obj.cast<py::str>().cast<QString>();
//            else
//                throw QString("Module does not contain a docstring");
        }

        // Check functions

        {
            if (!metadata_values.count("handleQuery"))
                throw QString("Modules does not contain a function definition for 'handleQuery'");

            if (py::len(metadata_values.at("handleQuery")) != 1)
                throw QString("handleQuery function definition does not take exactly one argument");
        }

        /*
         * Check/get optional metadata
         */

        {
            map<Target, QStringList&> zip{
                {Target::AUTHORS,   d->spec.authors},
                {Target::EXEC_DEPS, d->spec.executableDependecies},
                {Target::PY_DEPS,   d->spec.pythonDependecies},
                {Target::TRIGGERS,  d->spec.triggers}
            };
            for (const auto &pair : zip) {
                auto targetName = targetNames[pair.first];
                if (metadata_values.count(targetName)){
                    obj = metadata_values[targetName];
                    if (py::isinstance(obj, ast.attr("List"))) {
                        py::list list = obj.attr("elts").cast<py::list>();
                        for (const py::handle item : list) {
                            if (py::isinstance(item, ast.attr("Str")))
                                pair.second << item.attr("s").cast<py::str>().cast<QString>();
                            else
                                throw QString("%1 list contains non string values").arg(targetName);
                        }
                    }
                    else if (py::isinstance(obj, ast.attr("Str"))){
                        pair.second << obj.attr("s").cast<py::str>().cast<QString>();
                    }
                    else
                        throw QString("%1 is not list or string").arg(targetName);
                }
            }
        }

        /*
         * Check/get optional metadata
         */

        {

            for (const auto& exec : d->spec.executableDependecies)
                if (QStandardPaths::findExecutable(exec).isNull())
                    d->errorString = QString("No '%1' in $PATH.").arg(exec);

            py::module importlib = py::module::import("importlib");
            py::module importlib_util = py::module::import("importlib.util");
            for (const auto& dep : d->spec.pythonDependecies)
                if (!importlib_util.attr("find_spec")(QString(dep)).is_none())
                    d->errorString = QString("Could not locate python module '%1'.").arg(dep);

            if (!d->errorString.isNull()){
                INFO << d->errorString;
                d->state = State::MissingDeps;
                return;
            }
        }

        d->state = State::Unloaded;
    }
    catch(const QString &error)
    {
        d->errorString = error;
        WARN << QString("[%1] %2").arg(d->id).arg(d->errorString);
        d->state = State::InvalidMetadata;
    }
    catch(const std::exception &e)
    {
        d->errorString = e.what();
        WARN << QString("[%1] %2").arg(d->id).arg(d->errorString);
        d->state = State::InvalidMetadata;
    }
}


/** ***************************************************************************/
Python::PythonModuleV1::~PythonModuleV1() {
    unload();
}


/** ***************************************************************************/
void Python::PythonModuleV1::load(){

    if (d->state == State::Loaded || d->state == State::InvalidMetadata)
        return;

    py::gil_scoped_acquire acquire;

    try
    {
        DEBG << "Loading" << d->path;

        py::module importlib = py::module::import("importlib");
        py::module importli_util = py::module::import("importlib.util");
        py::object spec = importli_util.attr("spec_from_file_location")(QString("albert.%1").arg(d->id), d->sourceFilePath); // Prefix to avoid conflicts
        d->module = importli_util.attr("module_from_spec")(spec);
        spec.attr("loader").attr("exec_module")(d->module);

        // Call init function, if exists
        if (py::hasattr(d->module, "initialize"))
            if (py::isinstance<py::function>(d->module.attr("initialize")))
                d->module.attr("initialize")();
    }
    catch(const std::exception &e)
    {
        d->errorString = e.what();
        WARN << QString("[%1] %2.").arg(QFileInfo(d->path).fileName()).arg(d->errorString);
        d->module = py::object();
        d->state = State::Error;
        return;
    }

    d->state = State::Loaded;
}


/** ***************************************************************************/
void Python::PythonModuleV1::unload(){

    if (d->state == State::Unloaded)
        return;

    if (d->state == State::Loaded) {

        DEBG << "Unloading" << d->path;

        py::gil_scoped_acquire acquire;

        try {
            // Call fini function, if exists
            if (py::hasattr(d->module, "finalize"))
                if (py::isinstance<py::function>(d->module.attr("finalize")))
                    d->module.attr("finalize")();

            // Dereference module, unloads hopefully
            d->module = py::object();
        } catch(std::exception const &e) {
            WARN << QString("[%1] %2.").arg(QFileInfo(d->path).fileName()).arg(e.what());
        }
    }

    d->errorString.clear();
    d->state = State::Unloaded;
}


/** ***************************************************************************/
void Python::PythonModuleV1::handleQuery(Query *query) const {

    py::gil_scoped_acquire acquire;

    try {
        vector<pair<shared_ptr<Core::Item>,uint>> results;
        py::object pythonResult = py::function(d->module.attr("handleQuery"))(query);

        if ( !query->isValid() )
            return;

        if (py::isinstance<py::list>(pythonResult)) {

            py::list list(pythonResult);
            for(py::size_t i = 0; i < py::len(pythonResult); ++i) {
                py::object elem = list[i];
                results.emplace_back(elem.cast<shared_ptr<StandardItem>>(), 0);
            }

            query->addMatches(std::make_move_iterator(results.begin()),
                              std::make_move_iterator(results.end()));
        }

        if (py::isinstance<Item>(pythonResult)) {
            query->addMatch(pythonResult.cast<shared_ptr<StandardItem>>());
        }
    }
    catch(const exception &e)
    {
        WARN << QString("[%1] %2.").arg(d->id).arg(e.what());
    }
}


/** ***************************************************************************/
Python::PythonModuleV1::State Python::PythonModuleV1::state() const { return d->state; }
QString Python::PythonModuleV1::errorString() const { return d->errorString; }

QString Python::PythonModuleV1::path() const { return d->path; }
QString Python::PythonModuleV1::sourcePath() const { return d->sourceFilePath; }

QString Python::PythonModuleV1::id() const { return d->id; }
QString Python::PythonModuleV1::name() const { return d->spec.name; }
QString Python::PythonModuleV1::description() const { return d->spec.description; }
QString Python::PythonModuleV1::version() const { return d->spec.version; }
QStringList Python::PythonModuleV1::authors() const { return d->spec.authors; }
QStringList Python::PythonModuleV1::executableDependecies() const { return d->spec.executableDependecies; }
QStringList Python::PythonModuleV1::pythonDependecies() const { return d->spec.pythonDependecies; }
QStringList Python::PythonModuleV1::triggers() const { return d->spec.triggers; }



