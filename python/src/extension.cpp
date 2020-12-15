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
#include <QTableView>
#include <QUrl>
#include <memory>
#include "xdg/iconlookup.h"
#include "albert/query.h"
#include "albert/item.h"
#include "albert/action.h"
#include "albert/util/standardactions.h"
#include "albert/util/standarditem.h"
#include "extension.h"
#include "modulesmodel.h"
#include "configwidget.h"
#include "cast_specialization.h"
Q_LOGGING_CATEGORY(qlc, "python")
#define DEBG qCDebug(qlc,).noquote()
#define INFO qCInfo(qlc,).noquote()
#define WARN qCWarning(qlc,).noquote()
#define CRIT qCCritical(qlc,).noquote()
using namespace std;
using namespace Core;
namespace py = pybind11;

namespace Python {

const constexpr char* MODULES_DIR = "modules";
const constexpr char* CFG_ENABLEDMODS = "enabled_modules";

/*
 * Module definition
 */

#include <pybind11/stl.h>

PYBIND11_EMBEDDED_MODULE(albert, m)
{
    /*
     * 0.4
     */

    m.doc() = "pybind11 example module";

    py::class_<Core::Query, std::unique_ptr<Query, py::nodelete>> query(m, "Query", "The query object to handle for a user input");
    query.def_property_readonly("string", &Query::string);
    query.def_property_readonly("rawString", &Query::rawString);
    query.def_property_readonly("trigger", &Query::trigger);
    query.def_property_readonly("isTriggered", &Query::isTriggered);
    query.def_property_readonly("isValid", &Query::isValid);
    query.def("disableSort", &Query::disableSort);

    py::class_<Action, shared_ptr<Action>>(m, "ActionBase", "An abstract action")
            ;

    py::class_<Item, shared_ptr<Item>> iitem(m, "ItemBase", "An abstract item")
            ;

    /* This is a bit more evolved. In this case a piece of python code is injected into C++ code and
     * has to be handled with care. The GIL has to be locked whenever the code is touched, i.e. on
     * execution and deletion. Further exceptions thrown from python have to be catched. */
    py::class_<FuncAction, Action, shared_ptr<FuncAction>>(m, "FuncAction", "Executes the callable")
            .def(py::init(
                [](QString text, const py::object& callable) {
                    return shared_ptr<FuncAction>(

                        new FuncAction(move(text), [callable](){
                            py::gil_scoped_acquire acquire;
                            try{
                                callable();
                            } catch (exception &e) {
                                WARN << e.what();
                            }
                        }),

                        [=](FuncAction *funcAction) {
                            py::gil_scoped_acquire acquire;
                            delete funcAction;
                        }

                    );
                 }),
            py::arg("text"),
            py::arg("callable"))
            ;

    py::class_<ClipAction, Action, shared_ptr<ClipAction>>(m, "ClipAction", "Copies to clipboard")
            .def(py::init<QString, QString>(),
                 py::arg("text"),
                 py::arg("clipboardText"))
            ;

    py::class_<UrlAction, Action, shared_ptr<UrlAction>>(m, "UrlAction", "Opens a URL")
            .def(py::init<QString, QString>(),
                 py::arg("text"),
                 py::arg("url"))
            ;

    py::class_<ProcAction, Action, shared_ptr<ProcAction>>(m, "ProcAction", "Runs a process")
            .def(py::init<QString, QStringList, QString>(),
                 py::arg("text"),
                 py::arg("commandline"),
                 py::arg("cwd") = QString())
            ;

    py::class_<TermAction, Action, shared_ptr<TermAction>>pyTermAction(m, "TermAction", "Runs a command in terminal");
    py::enum_<TermAction::CloseBehavior>(pyTermAction, "CloseBehavior")
            .value("CloseOnSuccess", TermAction::CloseBehavior::CloseOnSuccess)
            .value("CloseOnExit", TermAction::CloseBehavior::CloseOnExit)
            .value("DoNotClose", TermAction::CloseBehavior::DoNotClose)
            .export_values()
            ;
    pyTermAction.def(py::init(
                [](QString text, list<QString> commandline, QString workdir) {
                    return std::make_shared<TermAction>(move(text), QStringList::fromStdList(commandline), move(workdir));
                }
            ),
            py::arg("text"),
            py::arg("commandline"),
            py::arg("cwd") = QString())
            ;
    pyTermAction.def(
                py::init<QString, QString, TermAction::CloseBehavior, QString>(),
                py::arg("text"),
                py::arg("script"),
                py::arg("behavior") = TermAction::CloseBehavior::CloseOnSuccess,
                py::arg("cwd") = QString()
            );

    py::enum_<Item::Urgency>(iitem, "Urgency")
            .value("Alert", Item::Urgency::Alert)
            .value("Notification", Item::Urgency::Notification)
            .value("Normal", Item::Urgency::Normal)
            .export_values()
            ;

    py::class_<StandardItem, Item, shared_ptr<StandardItem>>(m, "Item", "A result item")
            .def(py::init<QString, QString, QString, QString, ActionList, QString, Item::Urgency>(),
                 py::arg("id") = QString(),
                 py::arg("icon") = QString(":python_module"),
                 py::arg("text") = QString(),
                 py::arg("subtext") = QString(),
                 py::arg("actions") = vector<shared_ptr<Action>>(),
                 py::arg("completion") = QString(),
                 py::arg("urgency") = Item::Urgency::Normal
            )
            .def_property("id", &StandardItem::id, &StandardItem::setId)
            .def_property("icon", &StandardItem::iconPath, &StandardItem::setIconPath)
            .def_property("text", &StandardItem::text, &StandardItem::setText)
            .def_property("subtext", &StandardItem::subtext, &StandardItem::setSubtext)
            .def_property("actions", &StandardItem::actions, static_cast<void (StandardItem::*)(const ActionList&)>(&StandardItem::setActions))
            .def_property("completion", &StandardItem::completion, &StandardItem::setCompletion)
            .def_property("urgency", &StandardItem::urgency, &StandardItem::setUrgency)
            .def("addAction", static_cast<void (StandardItem::*)(const std::shared_ptr<Action> &)>(&StandardItem::addAction))
                ;

    m.def("debug",    [](const py::object &obj){ DEBG << py::str(obj).cast<QString>(); });
    m.def("info",     [](const py::object &obj){ INFO << py::str(obj).cast<QString>(); });
    m.def("warning",  [](const py::object &obj){ WARN << py::str(obj).cast<QString>(); });
    m.def("critical", [](const py::object &obj){ CRIT << py::str(obj).cast<QString>(); });

    m.def("iconLookup", static_cast<QString (*)(std::list<QString>,QString)>(&XDG::IconLookup::iconPath),
          py::arg("iconNames"), py::arg("themeName") = QString());

    m.def("iconLookup", static_cast<QString (*)(QString,QString)>(&XDG::IconLookup::iconPath),
          py::arg("iconName"), py::arg("themeName") = QString());

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
    QFileSystemWatcher extensionDirectoryWatcher;
    QFileSystemWatcher sourcesWatcher;
    QStringList enabledModules;
};


/** ***************************************************************************/
Python::Extension::Extension()
    : Core::Extension("org.albert.extension.python"), // Must match the id in metadata
      Core::QueryHandler(Core::Plugin::id()),
      d(new Private) {

    /*
     * The python interpreter is never unloaded once it has been loaded. This is working around the
     * ugly segfault that occur when third-party libraries have been loaded an the interpreter is
     * finalized and initialized.
     */
    if ( !Py_IsInitialized() )
        py::initialize_interpreter(false);
    d->release.reset(new py::gil_scoped_release);

    d->enabledModules = settings().value(CFG_ENABLEDMODS).toStringList();

    if ( !dataLocation().exists(MODULES_DIR) )
        dataLocation().mkdir(MODULES_DIR);

    // Watch the modules directories for changes
    for (const QString &dataDir : QStandardPaths::locateAll(QStandardPaths::DataLocation,
                                                            Core::Plugin::id(),
                                                            QStandardPaths::LocateDirectory) ) {
        QDir dir{dataDir};
        if (dir.cd(MODULES_DIR))
            d->extensionDirectoryWatcher.addPath(dir.path());
    }

    connect(&d->extensionDirectoryWatcher, &QFileSystemWatcher::directoryChanged,
            this, &Extension::reloadModules);

    connect(&d->sourcesWatcher, &QFileSystemWatcher::fileChanged,
            this, &Extension::reloadModules);

    reloadModules();

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
                 && module->triggers().contains(query->trigger()) ) {
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
        retval << module->triggers();
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
void Python::Extension::reloadModules() {

    // Reset

    d->modules.clear();
    d->sourcesWatcher.removePaths(d->sourcesWatcher.files());

    // Get all module source paths

    QStringList paths;
    for (const QString &dataDir : QStandardPaths::locateAll(QStandardPaths::DataLocation,
                                                            Core::Plugin::id(),
                                                            QStandardPaths::LocateDirectory) ) {
        QDir dir{dataDir};
        if (dir.cd(MODULES_DIR)) {
            for(const QFileInfo &fileInfo : dir.entryInfoList(QDir::Dirs|QDir::NoDotAndDotDot)) {
                QDir dir{fileInfo.filePath()};
                if (dir.exists("__init__.py"))
                    paths.append(dir.absolutePath());
            }
            for(const QFileInfo &fileInfo : dir.entryInfoList({"*.py"}, QDir::Files))
                paths.append(fileInfo.absoluteFilePath());
        }
    }

    // Read metada, ignore dup ids, load id enabled, source file watcher

    for(const QString &path : paths){
        try {
            auto module = make_unique<PythonModuleV1>(path);

            // Skip if this id already exists
            if ( find_if(d->modules.begin(), d->modules.end(),
                         [&module](const unique_ptr<PythonModuleV1> &rhs){return module->id() == rhs->id();})
                 != d->modules.end() )
                continue;

            if (d->enabledModules.contains(module->id()))
                module->load();

            d->sourcesWatcher.addPath(module->sourcePath());
            d->modules.emplace_back(move(module));

        } catch (const std::exception &e) {
            WARN << e.what() << path;
        }
    }

    std::sort(d->modules.begin(), d->modules.end(),
              [](auto& lhs, auto& rhs){ return 0 > lhs->name().localeAwareCompare(rhs->name()); });

    emit modulesChanged();

}


