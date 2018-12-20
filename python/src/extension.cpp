// Copyright (c) 2017-2018 Manuel Schneider

#include <pybind11/embed.h>
#include <pybind11/stl.h>
#include "pythonmodulev1.h"
#include <QClipboard>
#include <QDesktopServices>
#include <QDirIterator>
#include <QFileSystemWatcher>
#include <QMessageLogger>
#include <QMutexLocker>
#include <QPointer>
#include <QStandardPaths>
#include <QUrl>
#include <memory>
#include "xdg/iconlookup.h"
#include "core/query.h"
#include "core/item.h"
#include "core/action.h"
#include "util/standardactions.h"
#include "util/standarditem.h"
#include "extension.h"
#include "modulesmodel.h"
#include "configwidget.h"
#include "cast_specialization.h"
using namespace std;
using namespace Core;
namespace py = pybind11;

Q_LOGGING_CATEGORY(qlc_python, "python")
#define DEBUG qCDebug(qlc_python).noquote()
#define INFO qCInfo(qlc_python).noquote()
#define WARNING qCWarning(qlc_python).noquote()
#define CRITICAL qCCritical(qlc_python).noquote()

namespace Python {

const constexpr char* MODULES_DIR = "modules";
const constexpr char* CFG_ENABLEDMODS = "enabled_modules";

/*
 * Module definition
 */

#include <pybind11/stl.h>

PYBIND11_EMBEDDED_MODULE(albertv0, m)
{
    /*
     * 0.1
     */

    m.doc() = "pybind11 example module";

    py::class_<Core::Query, std::unique_ptr<Query, py::nodelete>>(m, "Query", "The query object to handle for a user input")
            .def_property_readonly("string", &Query::string)
            .def_property_readonly("rawString", &Query::rawString)
            .def_property_readonly("trigger", &Query::trigger)
            .def_property_readonly("isTriggered", &Query::isTriggered)
            .def_property_readonly("isValid", &Query::isValid)
            ;

    py::class_<Action, shared_ptr<Action>>(m, "ActionBase", "An abstract action")
            ;

    py::class_<Item, shared_ptr<Item>> iitem(m, "ItemBase", "An abstract item")
            ;

    /* This is a bit more evolved. In this case a piece of python code is injected into C++ code and
     * has to be handled with care. The GIL has to be locked whenever the code is touched, i.e. on
     * execution and deletion. Further exceptions thrown from python have to be catched. */
    py::class_<FuncAction, Action, shared_ptr<FuncAction>>(m, "FuncAction", "Executes the callable")
            .def(py::init([](QString text, const py::object& callable) {
                return shared_ptr<FuncAction>(
                    new FuncAction(move(text), [callable](){
                        py::gil_scoped_acquire acquire;
                        try{
                            callable();
                        } catch (exception &e) {
                            WARNING << e.what();
                        }
                    }),
                    [=](FuncAction *funcAction) {
                        py::gil_scoped_acquire acquire;
                        delete funcAction;
                    }
                );
            }), py::arg("text"), py::arg("callable"))
            ;

    py::class_<ClipAction, Action, shared_ptr<ClipAction>>(m, "ClipAction", "Copies to clipboard")
            .def(py::init<QString, QString>(), py::arg("text"), py::arg("clipboardText"))
            ;

    py::class_<UrlAction, Action, shared_ptr<UrlAction>>(m, "UrlAction", "Opens a URL")
            .def(py::init<QString, QString>(), py::arg("text"), py::arg("url"))
            ;

    py::class_<ProcAction, Action, shared_ptr<ProcAction>>(m, "ProcAction", "Runs a process")
            .def(py::init([](QString text, list<QString> commandline, QString workdir) {
                return std::make_shared<ProcAction>(move(text), QStringList::fromStdList(commandline), move(workdir));
            }), py::arg("text"), py::arg("commandline"), py::arg("cwd") = QString())
            ;

    py::class_<TermAction, Action, shared_ptr<TermAction>>pyTermAction(m, "TermAction", "Runs a command in terminal");

    py::enum_<TermAction::CloseBehavior>(pyTermAction, "CloseBehavior")
            .value("CloseOnSuccess", TermAction::CloseBehavior::CloseOnSuccess)
            .value("CloseOnExit", TermAction::CloseBehavior::CloseOnExit)
            .value("DoNotClose", TermAction::CloseBehavior::DoNotClose)
            .export_values()
            ;

    pyTermAction.def(py::init([](QString text, list<QString> commandline, QString workdir, bool shell, TermAction::CloseBehavior behavior) {
                return std::make_shared<TermAction>(move(text), QStringList::fromStdList(commandline), move(workdir), shell, behavior);
            }), py::arg("text"), py::arg("commandline"), py::arg("cwd") = QString(), py::arg("shell") = true, py::arg("behavior") = TermAction::CloseBehavior::CloseOnSuccess)
            ;

    py::enum_<Item::Urgency>(iitem, "Urgency")
            .value("Alert", Item::Urgency::Alert)
            .value("Notification", Item::Urgency::Notification)
            .value("Normal", Item::Urgency::Normal)
            .export_values()
            ;

    py::class_<StandardItem, Item, shared_ptr<StandardItem>>(m, "Item", "A result item")
            .def(py::init<QString,QString,QString,QString,QString,Item::Urgency,vector<shared_ptr<Action>>>(),
                 py::arg("id") = QString(),
                 py::arg("icon") = QString(":python_module"),
                 py::arg("text") = QString(),
                 py::arg("subtext") = QString(),
                 py::arg("completion") = QString(),
                 py::arg("urgency") = Item::Urgency::Normal,
                 py::arg("actions") = vector<shared_ptr<Action>>())
            .def_property("id", &StandardItem::id, &StandardItem::setId)
            .def_property("icon", &StandardItem::iconPath, &StandardItem::setIconPath)
            .def_property("text", &StandardItem::text, &StandardItem::setText)
            .def_property("subtext", &StandardItem::subtext, &StandardItem::setSubtext)
            .def_property("completion", &StandardItem::completion, &StandardItem::setCompletion)
            .def_property("urgency", &StandardItem::urgency, &StandardItem::setUrgency)
            .def("addAction", static_cast<void (StandardItem::*)(const std::shared_ptr<Action> &)>(&StandardItem::addAction))
            ;

    m.def("debug", [](const py::object &obj){ DEBUG << py::str(obj).cast<QString>(); });
    m.def("info", [](const py::object &obj){ INFO << py::str(obj).cast<QString>(); });
    m.def("warning", [](const py::object &obj){ WARNING << py::str(obj).cast<QString>(); });
    m.def("critical", [](const py::object &obj){ CRITICAL << py::str(obj).cast<QString>(); });

    m.def("iconLookup", [](const py::str &str){ return XDG::IconLookup::iconPath(str.cast<QString>()); });

    /*
     * 0.2
     */

    m.def("configLocation", [](){ return QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation); });
    m.def("dataLocation", [](){ return QStandardPaths::writableLocation(QStandardPaths::AppDataLocation); });
    m.def("cacheLocation", [](){ return QStandardPaths::writableLocation(QStandardPaths::CacheLocation); });


}

}

class Python::Private
{
public:
    unique_ptr<py::gil_scoped_release> release;
    QPointer<ConfigWidget> widget;
    vector<unique_ptr<PythonModuleV1>> modules;
    QFileSystemWatcher fileSystemWatcher;
    QStringList enabledModules;
};


/** ***************************************************************************/
Python::Extension::Extension()
    : Core::Extension("org.albert.extension.python"), // Must match the id in metadata
      Core::QueryHandler(Core::Plugin::id()),
      d(new Private) {

    /* The python interpreter is never unloaded once it has been loaded. This is working around the
     * ugly segfault that occur when third-party libraries have been loaded an the interpreter is
     * finalized and initialized. */
    if ( !Py_IsInitialized() )
        py::initialize_interpreter(false);

    d->release.reset(new py::gil_scoped_release);

    d->enabledModules = settings().value(CFG_ENABLEDMODS).toStringList();

    if ( !dataLocation().exists(MODULES_DIR) )
        dataLocation().mkdir(MODULES_DIR);

    // Load the modules
    for (const QString &pluginDir : QStandardPaths::locateAll(QStandardPaths::DataLocation,
                                                              Core::Plugin::id(),
                                                              QStandardPaths::LocateDirectory) ) {
        QString extensionDir = QDir(pluginDir).filePath(MODULES_DIR);
        if ( QFile::exists(extensionDir) ) {
            try { // Append scriptsPath to sys.path
                py::gil_scoped_acquire acquire;
                py::module::import("sys").attr("path").cast<py::list>().append(extensionDir);
            } catch (exception &e) {
                throw e.what();
            }
            d->fileSystemWatcher.addPath(extensionDir);
            updateDirectory(extensionDir);
        }
    }

    connect(&d->fileSystemWatcher, &QFileSystemWatcher::directoryChanged,
            this, &Extension::updateDirectory);

    registerQueryHandler(this);
}



/** ***************************************************************************/
Python::Extension::~Extension() {
    d->modules.clear();
}



/** ***************************************************************************/
QWidget *Python::Extension::widget(QWidget *parent) {
    if (d->widget.isNull()) {
        d->widget = new ConfigWidget(parent);

        ModulesModel *model = new ModulesModel(this, d->widget->ui.tableView);
        d->widget->ui.tableView->setModel(model);

        connect(d->widget->ui.tableView, &QTableView::activated,
                this, [this](const QModelIndex &index){
            QDesktopServices::openUrl(QUrl(d->modules[static_cast<size_t>(index.row())]->path()));
        });
    }
    return d->widget;
}


/** ***************************************************************************/
void Python::Extension::handleQuery(Core::Query *query) const {
    if ( query->isTriggered() ) {
        for ( auto & module : d->modules ) {
            if ( d->enabledModules.contains(module->id())
                 && module->state() == PythonModuleV1::State::Loaded
                 && module->trigger() == query->trigger() ) {
                module->handleQuery(query);
                return;
            }
        }
    }
    else {
        for ( auto & module : d->modules ) {
            if ( d->enabledModules.contains(module->id())
                 && module->state() == PythonModuleV1::State::Loaded) {
                module->handleQuery(query);
                if ( !query->isValid() )
                    return;
            }
        }
    }
}


/** ***************************************************************************/
QStringList Python::Extension::triggers() const {
    QStringList retval;
    for ( auto &module : d->modules )
        retval << module->trigger();
    return retval;
}


/** ***************************************************************************/
const std::vector<std::unique_ptr<Python::PythonModuleV1> > &Python::Extension::modules() {
    return d->modules;
}


/** ***************************************************************************/
bool Python::Extension::isEnabled(Python::PythonModuleV1 &module) {
    return d->enabledModules.contains(module.id());
}


/** ***************************************************************************/
void Python::Extension::setEnabled(Python::PythonModuleV1 &module, bool enable) {
    if (enable)
        d->enabledModules.append(module.id());
    else
        d->enabledModules.removeAll(module.id());
    settings().setValue(CFG_ENABLEDMODS, d->enabledModules);
    enable ? module.load() : module.unload();
}


/** ***************************************************************************/
void Python::Extension::updateDirectory(const QString &path) {

    // Remove deleted modules, yes deletes in other dirs too whatever… it has been deleted
    for ( auto it = d->modules.begin(); it != d->modules.end(); ++it)
        if ( !QFile::exists((*it)->path()) )
             it = --d->modules.erase(it);

    // Add new modules
    QDirIterator dirIterator(path, QDir::Files|QDir::Dirs|QDir::NoDotAndDotDot, QDirIterator::NoIteratorFlags);
    while (dirIterator.hasNext()) {
        QString path = dirIterator.next();
        QFileInfo info = dirIterator.fileInfo();
        QString id = info.completeBaseName();

        if (id == "__pycache__")
            continue;

        // Skip non-python files, e.g. README.md
        if (info.isFile() && !path.endsWith(".py"))
             continue;

        // Skip if this id already exists
        if ( find_if(d->modules.begin(), d->modules.end(),
                     [&id](const unique_ptr<PythonModuleV1> &rhs){return id == rhs->id();})
             != d->modules.end() )
            continue;

        PythonModuleV1 *module = new PythonModuleV1(path);
        d->modules.emplace_back(module);
        if (d->enabledModules.contains(module->id()))
            module->load();
        connect(module, &PythonModuleV1::moduleChanged,
                this, &Extension::modulesChanged);
    }

    std::sort(d->modules.begin(), d->modules.end(),
              [](auto& lhs, auto& rhs){ return 0 > lhs->name().localeAwareCompare(rhs->name()); });

    emit modulesChanged();
}


