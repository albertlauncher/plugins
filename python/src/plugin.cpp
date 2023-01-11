// Copyright (c) 2022 Manuel Schneider

#include <pybind11/embed.h>
#include <pybind11/stl.h>
#include "cast_specialization.h"
#include "plugin.h"
#include "ui_configwidget.h"
#include <QDesktopServices>
#include <QDirIterator>
#include <QFileSystemWatcher>
#include <QLabel>
#include <QMessageBox>
#include <QProcessEnvironment>
#include <QStandardPaths>
#include <iostream>
#include <memory>
ALBERT_LOGGING
using namespace std;
using namespace albert;
using std::chrono::duration_cast;
using std::chrono::milliseconds;
using std::chrono::system_clock;
static const constexpr char *PLUGIN_DIR = "plugins";
static const char *CFG_WATCH_SOURCES = "watchSources";
static const bool DEF_WATCH_SOURCES = false;

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
static const char *ATTR_MD_MINPY       = "md_min_python";
static const char *ATTR_PLUGIN_CLASS   = "Plugin";
static const char *ATTR_INITIALIZE     = "initialize";
static const char *ATTR_FINALIZE       = "finalize";
static const char *ATTR_EXTENSIONS     = "extensions";

struct PyPluginMetaData : public albert::PluginMetaData
{
    QString min_python;
};

class PyQueryhandler : albert::QueryHandler
{
public:
//    PyQueryhandler(){ CRIT << "PyQueryhandler created";}
//    ~PyQueryhandler(){ CRIT << "PyQueryhandler destroyed";}
    using QueryHandler::QueryHandler;  // Inherit the constructors
    QString id() const override { PYBIND11_OVERRIDE_PURE(QString, QueryHandler, id); }
    QString name() const override { PYBIND11_OVERRIDE_PURE(QString, QueryHandler, name); }
    QString description() const override { PYBIND11_OVERRIDE_PURE(QString, QueryHandler, description); }

    virtual void handleQuery_(Query *query) const { PYBIND11_OVERRIDE_NAME(void, PyQueryhandler, "handleQuery", handleQuery_, query); }
    void handleQuery(Query &query) const override { handleQuery_(&query); }

    vector<shared_ptr<albert::Item>> fallbacks(const QString &string) const override { PYBIND11_OVERRIDE(vector<shared_ptr<albert::Item>>, QueryHandler, fallbacks, string); }
    QString synopsis() const override { PYBIND11_OVERRIDE(QString, QueryHandler, synopsis); }
    QString defaultTrigger() const override { PYBIND11_OVERRIDE(QString, QueryHandler, defaultTrigger); }
    bool allowTriggerRemap() const override { PYBIND11_OVERRIDE(bool, QueryHandler, allowTriggerRemap); }
};

//class PyIndexQueryHandler : albert::IndexQueryHandler {
//    using IndexQueryHandler::IndexQueryHandler;  // Inherit the constructors
//    QString id() const override { PYBIND11_OVERRIDE_PURE(QString, IndexQueryHandler, id); }
//    QString name() const override { PYBIND11_OVERRIDE_PURE(QString, IndexQueryHandler, name); }
//    QString description() const override { PYBIND11_OVERRIDE_PURE(QString, IndexQueryHandler, description); }
//    vector<albert::IndexItem> indexItems() const override { PYBIND11_OVERRIDE_PURE(std::vector<albert::IndexItem>, IndexQueryHandler, indexItems); }
////    vector<shared_ptr<albert::Item>> fallbacks(const QString &string) const override { PYBIND11_OVERRIDE(vector<shared_ptr<albert::Item>>, QueryHandler, fallbacks, string); }
//    QString synopsis() const override { PYBIND11_OVERRIDE(QString, IndexQueryHandler, synopsis); }
//    QString defaultTrigger() const override { PYBIND11_OVERRIDE(QString, IndexQueryHandler, defaultTrigger); }
//    bool allowTriggerRemap() const override { PYBIND11_OVERRIDE(bool, IndexQueryHandler, allowTriggerRemap); }

//};

static const uint majorInterfaceVersion = 0;
static const uint minorInterfaceVersion = 5;
PYBIND11_EMBEDDED_MODULE(albert, m)  // interface id 0.5
{
    using namespace albert;

    /*
     * In this case a piece of python code is injected into C++ code.
     * The GIL has to be locked whenever the code is touched, i.e. on
     * execution and deletion. Further exceptions thrown from python
     * have to be catched.
     */
    struct GilAwareFunctor {
        py::object callable;
        GilAwareFunctor(const py::object &callable) : callable(callable){}
        GilAwareFunctor(GilAwareFunctor&&) = default;
        GilAwareFunctor & operator=(GilAwareFunctor&&) = default;
        GilAwareFunctor(const GilAwareFunctor &other){
            py::gil_scoped_acquire acquire;
            callable = other.callable;
        }
        GilAwareFunctor & operator=(const GilAwareFunctor &other){
            py::gil_scoped_acquire acquire;
            callable = other.callable;
            return *this;
        }
        ~GilAwareFunctor(){
            py::gil_scoped_acquire acquire;
            callable = py::object();
        }
        void operator()() {
            py::gil_scoped_acquire acquire;
            try {
                callable();
            } catch (exception &e) {
                WARN << e.what();
            }
        }
    };
    py::class_<Action>(m, "Action")
        .def(py::init([](QString id, QString text, const py::object &callable) {
                 py::gil_scoped_acquire acquire;
                 return Action(::move(id), ::move(text), GilAwareFunctor(callable));
             }),
             py::arg("id"),
             py::arg("text"),
             py::arg("callable"));

    py::class_<Item, shared_ptr<Item>> item(m, "AbstractItem");

    py::class_<StandardItem, Item, shared_ptr<StandardItem>>(m, "Item")
        .def(py::init(py::overload_cast<QString,QString,QString,QString,QStringList,albert::Actions>(&StandardItem::make)),
             py::arg("id") = QString(),
             py::arg("text") = QString(),
             py::arg("subtext") = QString(),
             py::arg("completion") = QString(),
             py::arg("icon") = QStringList(),
             py::arg("actions") = vector<shared_ptr<Action>>())
        .def_property("id", &StandardItem::id, &StandardItem::setId)
        .def_property("text", &StandardItem::text, &StandardItem::setText)
        .def_property("subtext", &StandardItem::subtext, &StandardItem::setSubtext)
        .def_property("completion", &StandardItem::inputActionText, &StandardItem::setInputActionText)
        .def_property("icon", &StandardItem::iconUrls, &StandardItem::setIconUrls)
        .def_property("actions", &StandardItem::actions, &StandardItem::setActions);

    using Query = QueryHandler::Query;
    py::class_<Query>(m, "Query")
        .def_property_readonly("trigger", &Query::trigger, py::return_value_policy::reference)
        .def_property_readonly("string", &Query::string, py::return_value_policy::reference)
        .def_property_readonly("isValid", &Query::isValid, py::return_value_policy::reference)
        .def("add", py::overload_cast<const shared_ptr<Item> &>(&Query::add), py::return_value_policy::reference)
        .def("add", py::overload_cast<const vector<shared_ptr<Item>> &>(&Query::add), py::return_value_policy::reference);

    py::class_<Extension, shared_ptr<Extension>>(m, "Extension")
        .def("id", &Extension::id)
        .def("name", &Extension::name)
        .def("description", &Extension::description)
        ;

    py::class_<QueryHandler, Extension, PyQueryhandler, shared_ptr<QueryHandler>>(m, "QueryHandler")
        .def(py::init<>())
        .def("handleQuery", &QueryHandler::handleQuery, py::return_value_policy::reference)
        .def("fallbacks", &QueryHandler::fallbacks)
        .def("synopsis", &QueryHandler::synopsis)
        .def("defaultTrigger", &QueryHandler::defaultTrigger)
        .def("allowTriggerRemap", &QueryHandler::allowTriggerRemap)
        ;

//    py::class_<albert::IndexItem>(m, "IndexItem")
//        .def(py::init<shared_ptr<Item>,QString>(),
//             py::arg("item"),
//             py::arg("string"))
//        .def_readwrite("item", &albert::IndexItem::item)
//        .def_readwrite("string", &albert::IndexItem::string)
//        ;

//    py::class_<IndexQueryHandler, Extension, PyIndexQueryHandler, shared_ptr<IndexQueryHandler>>(m, "IndexQueryHandler")
//        .def(py::init<>())
//        .def("indexItems", &IndexQueryHandler::indexItems)
////        .def("fallbacks", &IndexQueryHandler::fallbacks)
//        .def("synopsis", &IndexQueryHandler::synopsis)
//        .def("defaultTrigger", &IndexQueryHandler::defaultTrigger)
//        .def("allowTriggerRemap", &IndexQueryHandler::allowTriggerRemap)
//        ;

//    py::class_<PyPlugin, PyPluginTrampoline, shared_ptr<PyPlugin>>(m, "Plugin")
//        .def(py::init<>())
//        .def("initialize", &PyPlugin::initialize)
//        .def("finalize", &PyPlugin::finalize)
//        .def("extensions", &PyPlugin::extensions)
//        ;

    m.def("debug", [](const py::object &obj) { DEBG << py::str(obj).cast<QString>(); });
    m.def("info", [](const py::object &obj) { INFO << py::str(obj).cast<QString>(); });
    m.def("warning", [](const py::object &obj) { WARN << py::str(obj).cast<QString>(); });
    m.def("critical", [](const py::object &obj) { CRIT << py::str(obj).cast<QString>(); });

    m.def("configLocation", []() { return QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation); });
    m.def("dataLocation", []() { return QStandardPaths::writableLocation(QStandardPaths::AppDataLocation); });
    m.def("cacheLocation", []() { return QStandardPaths::writableLocation(QStandardPaths::CacheLocation); });

    m.def("setClipboardText", &albert::setClipboardText,
          py::arg("text") = QString());

    m.def("openUrl", &albert::openUrl,
          py::arg("url") = QString());

    m.def("runDetachedProcess", &albert::runDetachedProcess,
          py::arg("cmdln") = QStringList(),
          py::arg("workdir") = QString());

    m.def("runTerminal", &albert::runTerminal,
          py::arg("script") = QString(),
          py::arg("workdir") = QString(),
          py::arg("close_on_exit") = false);

    m.def("sendTrayNotification", &albert::sendTrayNotification,
          py::arg("title") = QString(),
          py::arg("msg") = QString(),
          py::arg("ms") = 10000);
}



class PyPluginLoader : public albert::PluginLoader
{
public:
    PyPluginLoader(Plugin *provider, albert::ExtensionRegistry &registry, const QFileInfo &file_info)
        : PluginLoader(file_info.absoluteFilePath()), provider_(provider), registry_(registry)
    {
        if(!file_info.exists())
            throw runtime_error("File path does not exist");
        else if (file_info.isFile()){
            if (path.endsWith(".py"))
                source_path_ = path;
            else
                throw runtime_error("Path is not a python file");
        }
        else if (QFileInfo fi(QDir(path).filePath("__init__.py")); fi.exists() && fi.isFile())
            source_path_ = fi.absoluteFilePath();
        else
            throw runtime_error("Python package init file does not exist");

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

        set<QString> ast_classes;
        map<QString, py::object> ast_assignments;

        for (auto node : ast_root.attr("body")){

//            if (py::isinstance(node, ast.attr("FunctionDef")))
//                metadata_values.emplace(node.attr("name").cast<QString>(), node.attr("args").attr("args"));

            if (py::isinstance(node, ast.attr("ClassDef"))){
                ast_classes.emplace(node.attr("name").cast<QString>());
//                INFO << node.attr("bases").cast<py::str>().cast<QString>();
//                for (auto &n : node.attr("bases").cast<vector<py::object>>())
//                    INFO << n.attr("id").cast<QString>();
            }

            else if (py::isinstance(node, ast.attr("Assign"))){
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
                        }
                    }
                }
            }
        }

        if (py::object obj = ast.attr("get_docstring")(ast_root); py::isinstance<py::str>(obj))
            metadata_.long_description = obj.cast<py::str>().cast<QString>();

        metadata_.user = true;

        // Validate metadata

        if (metadata_.iid.isEmpty())
            throw runtime_error("Not an albert plugin");


        QStringList errors;
        QRegularExpression regex_version(R"R(^(\d)\.(\d)$)R");

        if (auto match = regex_version.match(metadata_.iid); !match.hasMatch())
            errors << QString("Invalid version format: '%1'. Expected <major>.<minor>.")
                          .arg(match.captured(0));
        else if (uint maj = match.captured(1).toUInt(); maj != majorInterfaceVersion)
            errors << QString("Incompatible major interface version. Expected %1, got %2")
                          .arg(majorInterfaceVersion).arg(maj);
        else if (uint min = match.captured(2).toUInt(); min > minorInterfaceVersion)
            errors << QString("Incompatible minor interface version. Up to %1 supported, got %2.")
                          .arg(minorInterfaceVersion).arg(min);

        if (!regex_version.match(metadata_.version).hasMatch())
            errors << "Invalid version scheme. Use '<version>.<patch>'.";

        if (!ast_classes.contains(ATTR_PLUGIN_CLASS))
            errors << "Module does not have the mandatory class 'Plugin'";

        QRegularExpression regex_id(R"R(\w+)R");
        if (!regex_id.match(metadata_.id).hasMatch())
            errors << QString("Invalid plugin id '%1'. Use [a-z0-9_].").arg(metadata_.id);

        if (metadata_.name.isEmpty())
            errors << "'name' must not be empty.";

        if (metadata_.description.isEmpty())
            errors << "'description' must not be empty.";

        // Finally set state based on errors

        if (errors.isEmpty())
            state_ = PluginState::Unloaded;
        else{
            WARN << QString("Plugin invalid: %1. (%2)").arg(errors.join(", "), path);
            state_info_ = errors.join(", ");
        }

        //        {
        //            if (!metadata_values.count("handleQuery"))
        //                sthrow QString("Modules does not contain a function definition for 'handleQuery'");
        //
        //            if (py::len(metadata_values.at("handleQuery")) != 1)
        //                sthrow QString("handleQuery function definition does not take exactly one argument");
        //        }
        //        {
        //            for (const auto& exec : spec.executableDependecies)
        //                if (QStandardPaths::findExecutable(exec).isNull())
        //                    errorString = QString("No '%1' in $PATH.").arg(exec);
        //
        //            if (!errorString.isNull()){
        //                INFO << errorString;
        //                state = State::MissingDeps;
        //                return;
        //            }
        //        }
    }

    ~PyPluginLoader() = default;

    Plugin *provider() const override { return provider_; }

    PyPluginMetaData const &metaData() const override { return metadata_; }

    QString iconUrl() const override { return ":python"; }

    QWidget *makeInfoWidget() const override
    {
        auto w = new PluginInfoWidget(*this);
        w->layout->addRow("Min Python version:", new QLabel(metadata_.min_python, w));
        return w;
    }

    void load() override
    {
        if (state_ == PluginState::Invalid)
            qFatal("Loaded an invalid plugin.");
        else if (state_ == PluginState::Loaded)
            return;

        py::gil_scoped_acquire acquire;

        try
        {
            for (const auto& exec : metadata_.binary_dependencies)
                if (QStandardPaths::findExecutable(exec).isNull())
                    throw runtime_error(QString("No '%1' in $PATH.").arg(exec).toStdString());

            try {

                // Import as __name__ = albert.package_name
                py::module importlib_util = py::module::import("importlib.util");
                py::object pyspec = importlib_util.attr("spec_from_file_location")(QString("albert.%1").arg(metadata_.id), source_path_); // Prefix to avoid conflicts
                module_ = importlib_util.attr("module_from_spec")(pyspec);

                // Set default md_id
                if (!py::hasattr(module_, ATTR_MD_ID))
                    module_.attr("md_id") = metadata_.id;

                // Execute module
                pyspec.attr("loader").attr("exec_module")(module_);

                // Instanciate plugin
                instance_ = module_.attr("Plugin")();

                state_ = PluginState::Loaded;
                state_info_.clear();

            } catch (py::error_already_set &e) {
                if (e.matches(PyExc_ModuleNotFoundError)) {
                    QString text("Looks like something is missing:\n\n");
                    text.append(e.what());
                    text.append("\n\nTry installing missing dependencies into albert site-packages?\n\nNote that you have to reload the plugin afterwards.");
                    auto b = QMessageBox::warning(nullptr, "Module not found", text,
                                                  QMessageBox::Yes|QMessageBox::No, QMessageBox::Yes);
                    if (b==QMessageBox::Yes)
                        provider_->installPackages(metadata_.runtime_dependencies);
                }
                throw e;
            }

            if (py::hasattr(instance_, ATTR_INITIALIZE))
                if (auto init = instance_.attr(ATTR_INITIALIZE); py::isinstance<py::function>(init))
                    init();

            if (py::isinstance<Extension>(instance_))
                if (auto *e = instance_.cast<shared_ptr<Extension>>().get())
                    registry_.add(e);

//            if (py::hasattr(instance_, ATTR_EXTENSIONS))
//                if (auto exts = instance_.attr(ATTR_EXTENSIONS); py::isinstance<py::function>(exts))
//                    for (auto *e : exts())
//                        registry_.add(e);

            return;
        } catch(const std::exception &e) {
            state_info_ = e.what();
        } catch(...) {
            state_info_ = "Unknown exception while loading";
        }
        state_ = PluginState::Unloaded;
        module_ = py::object();  // should at least delete the module
    }

    void unload() override
    {
        if (state_ == PluginState::Invalid)
            qFatal("Unloaded an invalid plugin.");
        else if (state_ == PluginState::Unloaded)
            return;

        py::gil_scoped_acquire acquire;

        try {

            if (py::isinstance<Extension>(instance_))
                if (auto *e = instance_.cast<shared_ptr<Extension>>().get())
                    registry_.remove(e);

            if (py::hasattr(instance_, ATTR_FINALIZE))
                if (auto fini = instance_.attr(ATTR_FINALIZE); py::isinstance<py::function>(fini))
                    fini();

//            if (py::hasattr(instance_, ATTR_EXTENSIONS))
//                if (auto exts = instance_.attr(ATTR_EXTENSIONS); py::isinstance<py::function>(exts))
//                    for (auto *e : exts())
//                        registry_.remove(e);

            instance_ = py::object();
            module_ = py::object();
            state_ = PluginState::Unloaded;
            state_info_.clear();
        } catch(std::exception const &e) {
            WARN << QString("Error while unloading '%1': %2.").arg(metadata_.id).arg(e.what());
        } catch(...) {
            WARN << QString("Unknown error while unloading '%1'").arg(metadata_.id);
        }
    }

    const QString &source_path() const { return source_path_; }

private:
    QString source_path_;
    pybind11::module module_;
    Plugin *provider_;
    albert::ExtensionRegistry &registry_;
    py::object instance_;
    PyPluginMetaData metadata_;
};


// ///////////////////////////////////////////////////////////////////////////////////////////// //

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

    QString packages_path = dataDir().filePath("site-packages");
    py::module::import("site").attr("addsitedir")(packages_path);

    // Create module dirs
    if (!dataDir().exists(PLUGIN_DIR))
        dataDir().mkdir(PLUGIN_DIR);

    using QSP = QStandardPaths;
    auto plugin_dirs = QSP::locateAll(QSP::AppDataLocation, id(), QSP::LocateDirectory);
    for (const QString &plugin_dir : plugin_dirs) {
        if (QDir dir{plugin_dir}; dir.cd(PLUGIN_DIR)) {
            DEBG << "Searching Python plugins in" << dir.absolutePath();
            for (const QFileInfo &file_info : dir.entryInfoList(QDir::Dirs|QDir::Files|QDir::NoDotAndDotDot)) {
                try {
                    auto &loader = plugins_.emplace_back(this, registry(), file_info);
                    DEBG << "Found valid Python plugin" << loader.path;
                } catch (const exception &e) {
                    DEBG << e.what() << file_info.filePath();
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
        connect(sources_watcher_.get(), &QFileSystemWatcher::fileChanged, [this](){
            for (auto &loader : plugins_)
                if (loader.state() == PluginState::Loaded ||
                    (loader.state() == PluginState::Unloaded && !loader.stateInfo().isEmpty())){
                    loader.unload();
                    loader.load();
                }
        });
//        if (!sources_watcher_->files().isEmpty())
//            sources_watcher_->removePaths(sources_watcher_->files());
        for (const auto &loader : plugins_)
            sources_watcher_->addPath(loader.source_path());
    }
    settings()->setValue(CFG_WATCH_SOURCES, val);
}

vector<PluginLoader*> Plugin::plugins()
{
    vector<PluginLoader*> plugins;
    for (auto &plugin : plugins_)
        plugins.emplace_back(&plugin);
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

    return w;
}

void Plugin::installPackages(const QStringList &package_names) const
{
//    py::gil_scoped_acquire gil;
//    std::vector<string> params{"install", "--disable-pip-version-check", "--upgrade", "--target", dataDir().filePath("site-packages").toStdString()};
//    for (const auto &pn : package_names)
//        params.push_back(pn.toStdString());
//    py::module::import("pip").attr("main")(py::cast(params));

//    INFO << "Installing python packages:" << package_names;
//    QStringList args;
//    args << "-m"<< "pip"<< "install"<< "--disable-pip-version-check"<< /*"--upgrade"<< */"--target" << dataDir().filePath("site-packages") << package_names;
//    QProcess p;
//    p.start("python3", args);
//    p.waitForFinished(-1);
//    if (auto err = p.readAllStandardError(); !err.isEmpty()){
//        WARN << "pip stderr:";
//        cout << qPrintable(err) << endl;
//    }
//    if (auto out = p.readAllStandardOutput(); !out.isEmpty()){
//        WARN << "pip stdout:";
//        cout << qPrintable(out) << endl;
//    }

    auto script = QString(R"R(python3 -m pip install --disable-pip-version-check --target "%1" %2; cd %1)R")
                  .arg(dataDir().filePath("site-packages"), package_names.join(" "));
    runTerminal(script);
}
