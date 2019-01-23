// Copyright (c) 2017-2018 Manuel Schneider

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
#include <QMutex>
#include <QProcess>
#include <QProcess>
#include <QRegularExpression>
#include <QVBoxLayout>
#include <functional>
#include <vector>
#include "cast_specialization.h"
#include "albert/query.h"
#include "albert/util/standarditem.h"
#include "xdg/iconlookup.h"
using namespace std;
using namespace Core;
namespace py = pybind11;

Q_LOGGING_CATEGORY(qlc_python_modulev1, "python.modulev1")
#define DEBUG qCDebug(qlc_python_modulev1).noquote()
#define INFO qCInfo(qlc_python_modulev1).noquote()
#define WARNING qCWarning(qlc_python_modulev1).noquote()
#define CRITICAL qCCritical(qlc_python_modulev1).noquote()



namespace {
uint majorInterfaceVersion = 0;
uint minorInterfaceVersion = 3;

enum Target { IID, NAME, VERSION, TRIGGER, AUTHOR, DEPS};
const QStringList targetNames = {
    "__iid__",
    "__prettyname__",
    "__version__",
    "__trigger__",
    "__author__",
    "__dependencies__"
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
        QString iid;
        QString prettyName;
        QString author;
        QString version;
        QString trigger;
        QString description;
        QStringList dependencies;

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

    d->spec.prettyName = d->id = fileInfo.completeBaseName();
    d->state = State::InvalidMetadata;

    readMetadata();
}

void Python::PythonModuleV1::readMetadata() {

    DEBUG << "Reading metadata of python module:" << QFileInfo(d->path).fileName();

    py::gil_scoped_acquire acquire;

    try {

        // Get the extension spec source code

        QFile file(d->sourceFilePath);
        if(!file.open(QIODevice::ReadOnly))
            throw QString("Cant open init file: %1").arg(d->sourceFilePath);
        QString source = QTextStream(&file).readAll();
        file.close();

        // Parse it with ast

        py::module ast = py::module::import("ast");
        py::object ast_root = ast.attr("parse")(source.toStdString());

        // Get all FunctionDef and Assign ast nodes

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

        // Check interface id

        QString targetName = targetNames[Target::IID];
        if (!metadata_values.count(targetName))
            throw QString("Module has no %1 specified").arg(targetName);

        py::object astStringType = py::module::import("ast").attr("Str");
        py::object obj = metadata_values[targetName];
        if (!py::isinstance(obj, astStringType))
            throw QString("%1 is not of type ast.Str").arg(targetName);

        d->spec.iid = obj.attr("s").cast<py::str>().cast<QString>();

        QRegularExpression re("^PythonInterface\\/v(\\d)\\.(\\d)$");
        QRegularExpressionMatch match = re.match(d->spec.iid);
        if (!match.hasMatch())
            throw QString("Invalid interface id: %1").arg(d->spec.iid);

        uint maj = match.captured(1).toUInt();
        if (maj != majorInterfaceVersion)
            throw QString("Incompatible major interface version. Expected %1, got %2").arg(majorInterfaceVersion).arg(maj);

        uint min = match.captured(2).toUInt();
        if (min > minorInterfaceVersion)
            throw QString("Incompatible minor interface version. Up to %1 supported, got %2").arg(minorInterfaceVersion).arg(min);

        // Check mandatory handleQuery

        if (!metadata_values.count("handleQuery"))
            throw QString("Modules does not contain a function definition for 'handleQuery'");

        if (py::len(metadata_values.at("handleQuery")) != 1)
            throw QString("handleQuery function definition does not take exactly one argument");

        // Extract mandatory metadata

        obj = ast.attr("get_docstring")(ast_root);
        if (py::isinstance<py::str>(obj))
            d->spec.description = obj.cast<py::str>().cast<QString>();
        else
            throw QString("Module does not contain a docstring");

        map<Target, QString&> zip{{Target::NAME, d->spec.prettyName},
                                  {Target::VERSION, d->spec.version},
                                  {Target::AUTHOR, d->spec.author}};
        for (const auto &pair : zip) {
            targetName = targetNames[pair.first];
            if (metadata_values.count(targetName)){
                obj = metadata_values[targetName];
                if (py::isinstance(obj, astStringType))
                    pair.second = obj.attr("s").cast<py::str>().cast<QString>();
                else
                    throw QString("%1 is not of type ast.Str").arg(targetName);
            }
            else
                throw QString("Module has no %1 specified").arg(targetName);
        }

        // Extract optional metadata

        targetName = targetNames[Target::TRIGGER];
        if (metadata_values.count(targetName)){
            obj = metadata_values[targetName];
            if (py::isinstance(obj, astStringType))
                d->spec.trigger = obj.attr("s").cast<py::str>().cast<QString>();
            else
                throw QString("%1 is not of type ast.Str").arg(targetName);
        }

        targetName = targetNames[Target::DEPS];
        if (metadata_values.count(targetName)) {
            py::list deps = metadata_values[targetName].attr("elts").cast<py::list>();
            for (const py::handle dep : deps) {
                if (py::isinstance(dep, astStringType))
                    d->spec.dependencies.append(dep.attr("s").cast<py::str>().cast<QString>());
                else
                    throw QString("Dependencies contain non string values");
            }
        }

        d->state = State::Unloaded;
    }
    catch(const QString &error)
    {
        d->errorString = error;
        WARNING << QString("[%1] %2").arg(d->id).arg(d->errorString);
        d->state = State::InvalidMetadata;
    }
    catch(const std::exception &e)
    {
        d->errorString = e.what();
        WARNING << QString("[%1] %2").arg(d->id).arg(d->errorString);
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
        DEBUG << "Loading" << d->path;

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
        WARNING << QString("[%1] %2.").arg(QFileInfo(d->path).fileName()).arg(d->errorString);
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

        DEBUG << "Unloading" << d->path;

        py::gil_scoped_acquire acquire;

        try
        {
            // Call fini function, if exists
            if (py::hasattr(d->module, "finalize"))
                if (py::isinstance<py::function>(d->module.attr("finalize")))
                    d->module.attr("finalize")();

            // Dereference module, unloads hopefully
            d->module = py::object();
        }
        catch(std::exception const &e)
        {
            WARNING << QString("[%1] %2.").arg(QFileInfo(d->path).fileName()).arg(e.what());
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
        py::function f = py::function(d->module.attr("handleQuery"));
        py::object pythonResult = f(query);

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
        WARNING << QString("[%1] %2.").arg(d->id).arg(e.what());
    }
}


/** ***************************************************************************/
Python::PythonModuleV1::State Python::PythonModuleV1::state() const { return d->state; }
const QString &Python::PythonModuleV1::errorString() const { return d->errorString; }
const QString &Python::PythonModuleV1::path() const { return d->path; }
const QString &Python::PythonModuleV1::sourcePath() const { return d->sourceFilePath; }
const QString &Python::PythonModuleV1::id() const { return d->id; }
const QString &Python::PythonModuleV1::name() const { return d->spec.prettyName; }
const QString &Python::PythonModuleV1::author() const { return d->spec.author; }
const QString &Python::PythonModuleV1::version() const { return d->spec.version; }
const QString &Python::PythonModuleV1::description() const { return d->spec.description; }
const QString &Python::PythonModuleV1::trigger() const { return d->spec.trigger; }
const QStringList &Python::PythonModuleV1::dependencies() const { return d->spec.dependencies; }



