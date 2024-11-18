// Copyright (c) 2017-2024 Manuel Schneider

#pragma once

#include "cast_specialization.hpp" // Has to be imported first
#include "trampolineclasses.hpp" // Has to be imported first

#include <QDir>
#include <albert/indexqueryhandler.h>
#include <albert/logging.h>
#include <albert/matcher.h>
#include <albert/notification.h>
#include <albert/plugin/applications.h>
#include <albert/plugininstance.h>
#include <albert/standarditem.h>
#include <albert/util.h>
using namespace albert;
using namespace std;
extern applications::Plugin *apps;


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


template<class T, class PyT>
struct TrampolineDeleter
{
    void operator()(T* t) const {
        auto *pt = dynamic_cast<PyT*>(t);
        if(pt)
            delete pt;
        else
            CRIT << "Dynamic cast in trampoline deleter failed. Memory leaked.";
    }
};


PYBIND11_EMBEDDED_MODULE(albert, m)
{
    using namespace albert;

    // ------------------------------------------------------------------------

    py::class_<
            PluginInstance, PyPI,
            unique_ptr<PluginInstance, TrampolineDeleter<PluginInstance, PyPI>>
            >(m, "PluginInstance")
        .def(py::init<vector<Extension*>>(), py::arg("extensions") = vector<Extension*>{})
        .def_property_readonly("id", [](PyPI *self){ return self->loader().metaData().id; })
        .def_property_readonly("name", [](PyPI *self){ return self->loader().metaData().name; })
        .def_property_readonly("description", [](PyPI *self){ return self->loader().metaData().description; })
        .def_property_readonly("cacheLocation", [](PyPI *self){ return self->pathlibCachePath(); })
        .def_property_readonly("configLocation", [](PyPI *self){ return self->pathlibConfigPath(); })
        .def_property_readonly("dataLocation", [](PyPI *self){ return self->pathlibDataPath(); })
        .def("readConfig", [](PyPI *self, QString key, py::object type){ return self->readConfig(key, type); })
        .def("writeConfig", [](PyPI *self, QString key, py::object value){ self->writeConfig(key, value); })
        .def("registerExtension", [](PyPI *self, Extension *e){ self->registerExtension(e); })
        .def("deregisterExtension", [](PyPI *self, Extension *e){ self->deregisterExtension(e); })
        ;

    // ------------------------------------------------------------------------

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

    py::class_<Query, unique_ptr<Query, py::nodelete>>(m, "Query")
        .def_property_readonly("trigger", &Query::trigger)
        .def_property_readonly("string", &Query::string)
        .def_property_readonly("isValid", &Query::isValid)
        .def("add", py::overload_cast<const shared_ptr<Item> &>(&Query::add))
        .def("add", py::overload_cast<const vector<shared_ptr<Item>> &>(&Query::add))
        ;

    py::class_<Match>(m, "Match")
        .def("__bool__", &Match::operator bool)
        .def("isMatch", &Match::isMatch)
        .def("isEmptyMatch", &Match::isEmptyMatch)
        .def("isExactMatch", &Match::isExactMatch)
        .def_property_readonly("score", &Match::score)
        ;

    py::class_<Matcher>(m, "Matcher")
        .def(py::init<const QString&>())
        .def("match", static_cast<Match(Matcher::*)(const QString&) const>(&Matcher::match))
        .def("match", static_cast<Match(Matcher::*)(const QStringList&) const>(&Matcher::match))
        .def("match", [](Matcher *self, py::args args){ return self->match(py::cast<QStringList>(args)); });
        ;

    // ------------------------------------------------------------------------

    py::class_<
            Extension, PyE<>,
            unique_ptr<Extension, py::nodelete>
            >(m, "Extension")
        .def_property_readonly("id", &Extension::id)
        .def_property_readonly("name", &Extension::name)
        .def_property_readonly("description", &Extension::description)
        ;

    // ------------------------------------------------------------------------

    py::class_<
            FallbackHandler, Extension, PyFQH<>,
            unique_ptr<FallbackHandler, TrampolineDeleter<FallbackHandler, PyFQH<>>>
            >(m, "FallbackHandler")
        .def(py::init_alias<const QString&, const QString&, const QString&>(),
             py::arg("id"),
             py::arg("name"),
             py::arg("description"))
        ;

    // ------------------------------------------------------------------------

    py::class_<
            TriggerQueryHandler, Extension, PyTQH<>,
            unique_ptr<TriggerQueryHandler, TrampolineDeleter<TriggerQueryHandler, PyTQH<>>>
            >(m, "TriggerQueryHandler")
        .def(py::init_alias<const QString&, const QString&, const QString&, const QString&, const QString&, bool, bool>(),
             py::arg("id"),
             py::arg("name"),
             py::arg("description"),
             py::arg("synopsis") = QString(),
             py::arg("defaultTrigger") = QString(),
             py::arg("allowTriggerRemap") = true,
             py::arg("supportsFuzzyMatching") = false)
        .def_property_readonly("synopsis", &TriggerQueryHandler::synopsis)
        .def_property_readonly("defaultTrigger", &TriggerQueryHandler::defaultTrigger)
        .def_property_readonly("allowTriggerRemap", &TriggerQueryHandler::allowTriggerRemap)
        .def_property_readonly("supportsFuzzyMatching", &TriggerQueryHandler::supportsFuzzyMatching)
        .def("handleTriggerQuery", &TriggerQueryHandler::handleTriggerQuery, py::arg("query"))
        ;

    // ------------------------------------------------------------------------

    py::class_<RankItem>(m, "RankItem")
        .def(py::init<shared_ptr<Item>,float>(), py::arg("item"), py::arg("score"))
        .def_readwrite("item", &RankItem::item)
        .def_readwrite("score", &RankItem::score)
        ;

    py::class_<
            GlobalQueryHandler, TriggerQueryHandler, PyGQH<>,
            unique_ptr<GlobalQueryHandler, TrampolineDeleter<GlobalQueryHandler, PyGQH<>>>
            >(m, "GlobalQueryHandler")
        .def(py::init_alias<const QString&, const QString&, const QString&, const QString&, const QString&, bool, bool>(),
             py::arg("id"),
             py::arg("name"),
             py::arg("description"),
             py::arg("synopsis") = QString(),
             py::arg("defaultTrigger") = QString(),
             py::arg("allowTriggerRemap") = true,
             py::arg("supportsFuzzyMatching") = false)
        .def("applyUsageScore", &GlobalQueryHandler::applyUsageScore)
        .def("handleGlobalQuery", &GlobalQueryHandler::handleGlobalQuery, py::arg("query"))
        ;

    // ------------------------------------------------------------------------

    py::class_<IndexItem>(m, "IndexItem")
        .def(py::init<shared_ptr<Item>,QString>(), py::arg("item"), py::arg("string"))
        .def_readwrite("item", &IndexItem::item)
        .def_readwrite("string", &IndexItem::string)
        ;

    py::class_<
            IndexQueryHandler, GlobalQueryHandler, PyIQH<>,
            unique_ptr<IndexQueryHandler, TrampolineDeleter<IndexQueryHandler, PyIQH<>>>
            >(m, "IndexQueryHandler")
        .def(py::init_alias<const QString&, const QString&, const QString&, const QString&, const QString&, bool>(),
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

    m.def("setClipboardText", &setClipboardText,
          py::arg("text") = QString());

    m.def("havePasteSupport", &havePasteSupport);

    m.def("setClipboardTextAndPaste", &setClipboardTextAndPaste,
          py::arg("text") = QString());

    m.def("openUrl", static_cast<void(*)(const QString &)>(&openUrl),
          py::arg("url") = QString());

    m.def("runDetachedProcess", &runDetachedProcess,
          py::arg("cmdln") = QStringList(),
          py::arg("workdir") = QString());

    auto runTerminal = [](const QString &s, const QString &w, bool c){

        // TODO Remove in v3.0

        QString script;

        if (!w.isEmpty())
        {
            WARN << "Parameter `workdir` is deprecated and will be removed in v3.0."
                 << "Prepend `cd <workdir>;` to your script";
            script = QString("cd %1; ").arg(w);
        }

        script.append(s);

        if (c)
        {
            WARN << "Parameter `close_on_exit` is deprecated and will be removed in v3.0."
                 << "Append `exec $SHELL;` to your script.";
            script.append(QString(" ; exec $SHELL"));
        }

        apps->runTerminal(script);

    };

    m.def("runTerminal", runTerminal,
          py::arg("script") = QString(),
          py::arg("workdir") = QString(),
          py::arg("close_on_exit") = false);

    py::class_<Notification>(m, "Notification")
        .def(py::init<const QString&, const QString&>(),
             py::arg("title") = QString(), py::arg("text") = QString())
        .def_property("title", &Notification::title, &Notification::setTitle)
        .def_property("text", &Notification::text, &Notification::setText)
        .def("send", &Notification::send)
        .def("dismiss", &Notification::dismiss)
        ;
}
