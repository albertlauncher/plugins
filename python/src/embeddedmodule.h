// Copyright (c) 2017-2023 Manuel Schneider

#pragma once
#include "cast_specialization.h" // Has to be imported first
#include "trampolineclasses.h" // Has to be imported first
#include "albert/albert.h"
#include "albert/extension/pluginprovider/plugininstance.h"
#include "albert/extension/queryhandler/globalqueryhandler.h"
#include "albert/extension/queryhandler/indexqueryhandler.h"
#include "albert/extension/queryhandler/item.h"
#include "albert/extension/queryhandler/standarditem.h"
#include "albert/extension/queryhandler/triggerqueryhandler.h"
#include "albert/logging.h"
#include <QDir>
using namespace albert;
using namespace std;

PYBIND11_EMBEDDED_MODULE(albert, m)
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
        GilAwareFunctor(const py::object &c) : callable(c){}
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
             py::arg("callable"))
        ;

    py::class_<Item, PyItemTrampoline, shared_ptr<Item>>(m, "Item")
        .def(py::init<>())
        .def_property_readonly("id", &Item::id)
        .def_property_readonly("text", &Item::text)
        .def_property_readonly("subtext", &Item::subtext)
        .def_property_readonly("inputActionText", &Item::inputActionText)
        .def_property_readonly("iconUrls", &Item::iconUrls)
        .def_property_readonly("actions", &Item::actions)
        ;

    py::class_<StandardItem, Item, shared_ptr<StandardItem>>(m, "StandardItem")
        .def(py::init(py::overload_cast<QString,QString,QString,QString,QStringList,vector<Action>>(&StandardItem::make)),
             py::arg("id") = QString(),
             py::arg("text") = QString(),
             py::arg("subtext") = QString(),
             py::arg("inputActionText") = QString(),
             py::arg("iconUrls") = QStringList(),
             py::arg("actions") = vector<Action>())
        .def_property("id", &StandardItem::id, &StandardItem::setId)
        .def_property("text", &StandardItem::text, &StandardItem::setText)
        .def_property("subtext", &StandardItem::subtext, &StandardItem::setSubtext)
        .def_property("inputActionText", &StandardItem::inputActionText, &StandardItem::setInputActionText)
        .def_property("iconUrls", &StandardItem::iconUrls, &StandardItem::setIconUrls)
        .def_property("actions", &StandardItem::actions, &StandardItem::setActions)
        ;

    // ------------------------------------------------------------------------

    py::class_<PluginInstance, PyPluginInstanceTrampoline<>, shared_ptr<PluginInstance>>(m, "PluginInstance", py::multiple_inheritance())
        .def(py::init<vector<Extension*>>(),
             py::arg("extensions") = vector<Extension*>{})
        .def_property_readonly("id", &PluginInstance::id)
        .def_property_readonly("name", &PluginInstance::name)
        .def_property_readonly("description", &PluginInstance::description)
        .def_property_readonly("cacheLocation", [](PyPluginInstanceTrampoline<> *self){ return self->pathlibCachePath(); }, py::return_value_policy::reference)
        .def_property_readonly("configLocation", [](PyPluginInstanceTrampoline<> *self){ return self->pathlibConfigPath(); }, py::return_value_policy::reference)
        .def_property_readonly("dataLocation", [](PyPluginInstanceTrampoline<> *self){ return self->pathlibDataPath(); }, py::return_value_policy::reference)
        ;

    // ------------------------------------------------------------------------

    py::class_<Extension, PyExtensionTrampoline<>, shared_ptr<Extension>>(m, "Extension")
        .def_property_readonly("id", &Extension::id)
        .def_property_readonly("name", &Extension::name)
        .def_property_readonly("description", &Extension::description)
        ;

    // ------------------------------------------------------------------------

    py::class_<FallbackHandler, Extension, PyFallbackHandlerTrampoline<>, shared_ptr<FallbackHandler>>(m, "FallbackHandler")
        .def(py::init<const QString&, const QString&, const QString&>(),
             py::arg("id"),
             py::arg("name"),
             py::arg("description"))
        ;

    // ------------------------------------------------------------------------

    using TriggerQuery = TriggerQueryHandler::TriggerQuery;
    py::class_<TriggerQuery>(m, "TriggerQuery")
        .def_property_readonly("trigger", &TriggerQuery::trigger)
        .def_property_readonly("string", &TriggerQuery::string)
        .def_property_readonly("isValid", &TriggerQuery::isValid)
        .def("add", py::overload_cast<const shared_ptr<Item> &>(&TriggerQuery::add))
        .def("add", py::overload_cast<const vector<shared_ptr<Item>> &>(&TriggerQuery::add))
        ;

    py::class_<TriggerQueryHandler, Extension, PyTriggerQueryHandlerTrampoline<>, shared_ptr<TriggerQueryHandler>>(m, "TriggerQueryHandler")
        .def(py::init<const QString&, const QString&, const QString&, const QString&, const QString&, bool, bool>(),
             py::arg("id"),
             py::arg("name"),
             py::arg("description"),
             py::arg("synopsis") = QString(),
             py::arg("defaultTrigger") = QString(),
             py::arg("allowTriggerRemap") = true,
             py::arg("supportsFuzzyMatching") = false)
        .def_property_readonly("synopsis", &TriggerQueryHandler::synopsis)
        .def_property_readonly("trigger", &TriggerQueryHandler::trigger)
        .def_property_readonly("defaultTrigger", &TriggerQueryHandler::defaultTrigger)
        .def_property_readonly("allowTriggerRemap", &TriggerQueryHandler::allowTriggerRemap)
        .def_property_readonly("supportsFuzzyMatching", &TriggerQueryHandler::supportsFuzzyMatching)
        ;

    // ------------------------------------------------------------------------

    using GlobalQuery = GlobalQueryHandler::GlobalQuery;
    py::class_<GlobalQuery>(m, "GlobalQuery")
        .def_property_readonly("string", &GlobalQuery::string)
        .def_property_readonly("isValid", &GlobalQuery::isValid)
        ;

    py::class_<RankItem>(m, "RankItem")
        .def(py::init<shared_ptr<Item>,float>(), py::arg("item"), py::arg("score"))
        .def_readwrite("item", &RankItem::item)
        .def_readwrite("score", &RankItem::score)
        ;

    py::class_<GlobalQueryHandler, TriggerQueryHandler, PyGlobalQueryHandlerTrampoline<>, shared_ptr<GlobalQueryHandler>>(m, "GlobalQueryHandler")
        .def(py::init<const QString&, const QString&, const QString&, const QString&, const QString&, bool, bool>(),
             py::arg("id"),
             py::arg("name"),
             py::arg("description"),
             py::arg("synopsis") = QString(),
             py::arg("defaultTrigger") = QString(),
             py::arg("allowTriggerRemap") = true,
             py::arg("supportsFuzzyMatching") = false)
        .def("applyUsageScore", &GlobalQueryHandler::applyUsageScore)
        ;

    // ------------------------------------------------------------------------

    py::class_<IndexItem>(m, "IndexItem")
        .def(py::init<shared_ptr<Item>,QString>(), py::arg("item"), py::arg("string"))
        .def_readwrite("item", &IndexItem::item)
        .def_readwrite("string", &IndexItem::string)
        ;

    py::class_<IndexQueryHandler, GlobalQueryHandler, PyIndexQueryHandlerTrampoline<>, shared_ptr<IndexQueryHandler>>(m, "IndexQueryHandler")
        .def(py::init<const QString&, const QString&, const QString&, const QString&, const QString&, bool>(),
             py::arg("id"),
             py::arg("name"),
             py::arg("description"),
             py::arg("synopsis") = QString(),
             py::arg("defaultTrigger") = QString(),
             py::arg("allowTriggerRemap") = true)
        .def("updateIndexItems", &IndexQueryHandler::updateIndexItems)
        .def("setIndexItems", &IndexQueryHandler::setIndexItems, py::arg("indexItems"))
        ;

    // ------------------------------------------------------------------------

    // Convenience classes

//    py::class_<TQHPI, TQHPIT, PluginInstance, TriggerQueryHandler, shared_ptr<TQHPI>>(m, "TriggerQueryHandlerPlugin", py::multiple_inheritance())
//        .def(py::init<const QString&, const QString&, bool, bool>(),
//             py::arg("synopsis") = QString(),
//             py::arg("defaultTrigger") = QString(),
//             py::arg("allowTriggerRemap") = true,
//             py::arg("supportsFuzzyMatching") = false)
//////        .def("handleTriggerQuery", &PyTriggerQueryHandlerPlugin<>::handleTriggerQuery)
//        ;

//    py::class_<PyGlobalQueryHandlerPlugin, PluginInstance, GlobalQueryHandler, shared_ptr<PyGlobalQueryHandlerPlugin>>(m, "GlobalQueryHandlerPlugin")
//        .def(py::init<const QString&, const QString&, bool, bool>(),
//             py::arg("synopsis") = QString(),
//             py::arg("defaultTrigger") = QString(),
//             py::arg("allowTriggerRemap") = true,
//             py::arg("supportsFuzzyMatching") = false)
//        ;

//    py::class_<PyIndexQueryHandlerPlugin, PluginInstance, PyIndexQueryHandlerTrampoline<>, shared_ptr<PyIndexQueryHandlerPlugin>>(m, "IndexQueryHandlerPlugin")
//        .def(py::init<const QString&, const QString&, bool>(),
//             py::arg("synopsis") = QString(),
//             py::arg("defaultTrigger") = QString(),
//             py::arg("allowTriggerRemap") = true)
//        ;

    // ------------------------------------------------------------------------

    m.def("debug", [](const py::object &obj) { DEBG << py::str(obj).cast<QString>(); });
    m.def("info", [](const py::object &obj) { INFO << py::str(obj).cast<QString>(); });
    m.def("warning", [](const py::object &obj) { WARN << py::str(obj).cast<QString>(); });
    m.def("critical", [](const py::object &obj) { CRIT << py::str(obj).cast<QString>(); });

    m.def("setClipboardText", &setClipboardText,
          py::arg("text") = QString());

    m.def("setClipboardTextAndPaste", &setClipboardTextAndPaste,
          py::arg("text") = QString());

    m.def("openUrl", static_cast<void(*)(const QString &)>(&openUrl),
          py::arg("url") = QString());

    m.def("runDetachedProcess", &runDetachedProcess,
          py::arg("cmdln") = QStringList(),
          py::arg("workdir") = QString());

    m.def("runTerminal", &runTerminal,
          py::arg("script") = QString(),
          py::arg("workdir") = QString(),
          py::arg("close_on_exit") = false);

    py::class_<Notification>(m, "Notification")
        .def(py::init<const QString&, const QString&, const QString&>(),
             py::arg("title"),
             py::arg("subtitle") = QString(),
             py::arg("text") = QString())
        ;
}



