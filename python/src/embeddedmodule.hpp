// Copyright (c) 2017-2024 Manuel Schneider

#pragma once

#include <pybind11/stl/filesystem.h>  // for automatic path conversion
#include "cast_specialization.hpp" // Has to be imported first
#include "trampolineclasses.hpp" // Has to be imported first

#include <QDir>
#include <albert/albert.h>
#include <albert/indexqueryhandler.h>
#include <albert/logging.h>
#include <albert/matcher.h>
#include <albert/notification.h>
#include <albert/plugin/applications.h>
#include <albert/plugininstance.h>
#include <albert/standarditem.h>
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
        .def(py::init<>())
        .def("id", [](PyPI *self){ return self->loader().metaData().id; })
        .def("name", [](PyPI *self){ return self->loader().metaData().name; })
        .def("description", [](PyPI *self){ return self->loader().metaData().description; })
        .def("cacheLocation", &PluginInstance::cacheLocation)
        .def("configLocation", &PluginInstance::configLocation)
        .def("dataLocation", &PluginInstance::dataLocation)
        .def("readConfig", [](PyPI *self, QString key, py::object type){ return self->readConfig(key, type); })
        .def("writeConfig", [](PyPI *self, QString key, py::object value){ self->writeConfig(key, value); })
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
        .def("id", &Item::id)
        .def("text", &Item::text)
        .def("subtext", &Item::subtext)
        .def("inputActionText", &Item::inputActionText)
        .def("iconUrls", &Item::iconUrls)
        .def("actions", &Item::actions)
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

    py::class_<MatchConfig>(m, "MatchConfig")
        .def(py::init<>())
        .def(py::init([](bool f, bool c, bool o, bool d, const QString &r) {
                 return MatchConfig{
                     .fuzzy=f,
                     .ignore_case=c,
                     .ignore_word_order=o,
                     .ignore_diacritics=d,
                     .separator_regex=r.isEmpty() ? default_separator_regex : QRegularExpression(r)
                 };
             }),
            py::arg("fuzzy") = false,
            py::arg("ignore_case") = true,
            py::arg("ignore_word_order") = true,
            py::arg("ignore_diacritics") = true,
            py::arg("separator_regex") = QString())
        .def_readwrite("fuzzy", &MatchConfig::fuzzy)
        .def_readwrite("ignore_case", &MatchConfig::ignore_case)
        .def_readwrite("ignore_word_order", &MatchConfig::ignore_word_order)
        .def_readwrite("ignore_diacritics", &MatchConfig::ignore_diacritics)
        .def_property("separator_regex",
                      [](const MatchConfig &self){ return self.separator_regex.pattern(); },
                      [](MatchConfig &self, const QString &pattern){ self.separator_regex.setPattern(pattern); })
        ;

    py::class_<Matcher>(m, "Matcher")
        .def(py::init<QString, MatchConfig>(), py::arg("string"), py::arg("config") = MatchConfig())
        .def("match", static_cast<Match(Matcher::*)(const QString&) const>(&Matcher::match))
        .def("match", static_cast<Match(Matcher::*)(const QStringList&) const>(&Matcher::match))
        .def("match", [](Matcher *self, py::args args){ return self->match(py::cast<QStringList>(args)); });
        ;

    py::class_<Match>(m, "Match")
        .def("__bool__", &Match::operator bool)
        .def("__float__", &Match::operator Match::Score)
        .def("isMatch", &Match::isMatch)
        .def("isEmptyMatch", &Match::isEmptyMatch)
        .def("isExactMatch", &Match::isExactMatch)
        .def_property_readonly("score", &Match::score)
        ;

    // ------------------------------------------------------------------------

    py::class_<
            Extension, PyE<>,
            unique_ptr<Extension, py::nodelete>
            >(m, "Extension")
        ;

    // ------------------------------------------------------------------------

    py::class_<
            TriggerQueryHandler, Extension, PyTQH<>,
            unique_ptr<TriggerQueryHandler, TrampolineDeleter<TriggerQueryHandler, PyTQH<>>>
            >(m, "TriggerQueryHandler")
        .def(py::init<>())
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
        .def(py::init<>())
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
        .def(py::init<>())
        .def("updateIndexItems", &IndexQueryHandler::updateIndexItems)
        .def("setIndexItems", &IndexQueryHandler::setIndexItems, py::arg("indexItems"))
        ;

    //------------------------------------------------------------------------

    py::class_<
            FallbackHandler, Extension, PyFQH<>,
            unique_ptr<FallbackHandler, TrampolineDeleter<FallbackHandler, PyFQH<>>>
            >(m, "FallbackHandler")
        .def(py::init<>())
        .def("fallbacks", &FallbackHandler::fallbacks)
        ;

    // ------------------------------------------------------------------------

    m.def("setClipboardText", &setClipboardText, py::arg("text"));
    m.def("setClipboardTextAndPaste", &setClipboardTextAndPaste, py::arg("text"));
    m.def("havePasteSupport", &havePasteSupport);

    // open conflicsts the built-in open. Use openFile.
    m.def("openFile", static_cast<void(*)(const QString &)>(&open), py::arg("path"));
    m.def("openUrl", &openUrl, py::arg("url"));

    m.def("runDetachedProcess", &runDetachedProcess,
          py::arg("cmdln"),
          py::arg("workdir") = QString());

    m.def("runTerminal", [](const QString &s){ apps->runTerminal(s); }, py::arg("script"));

    py::class_<Notification>(m, "Notification")
        .def(py::init<const QString&, const QString&>(),
             py::arg("title") = QString(), py::arg("text") = QString())
        .def_property("title", &Notification::title, &Notification::setTitle)
        .def_property("text", &Notification::text, &Notification::setText)
        .def("send", &Notification::send)
        .def("dismiss", &Notification::dismiss)
        ;
}
