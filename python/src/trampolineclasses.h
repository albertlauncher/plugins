// Copyright (c) 2017-2023 Manuel Schneider

#pragma once
#include "cast_specialization.h" // Has to be imported first
#include "albert/extension/pluginprovider/plugininstance.h"
#include "albert/extension/queryhandler/fallbackprovider.h"
#include "albert/extension/queryhandler/globalqueryhandler.h"
#include "albert/extension/queryhandler/indexqueryhandler.h"
#include "albert/extension/queryhandler/item.h"
#include "albert/extension/queryhandler/triggerqueryhandler.h"
#include "albert/logging.h"
#include <QDir>
using namespace albert;
using namespace std;


class PyItemTrampoline : Item
{
public:
    QString id() const override
    { PYBIND11_OVERRIDE_PURE(QString, Item, id, ); }

    QString text() const override
    { PYBIND11_OVERRIDE_PURE(QString, Item, text, ); }

    QString subtext() const override
    { PYBIND11_OVERRIDE_PURE(QString, Item, subtext, ); }

    QStringList iconUrls() const override
    { PYBIND11_OVERRIDE_PURE(QStringList, Item, iconUrls, ); }

    QString inputActionText() const override
    { PYBIND11_OVERRIDE(QString, Item, inputActionText, ); }

    vector<Action> actions() const override
    { PYBIND11_OVERRIDE(vector<Action>, Item, actions, ); }
};


template <class PluginInstanceBase = PluginInstance>
class PyPluginInstanceTrampoline : public PluginInstanceBase
{
public:
    PyPluginInstanceTrampoline(vector<Extension*> e = {}) : extensions_(e) {
        py::gil_scoped_acquire a;
        Path = py::module_::import("pathlib").attr("Path");
    }

    // Drop param
    void initialize(ExtensionRegistry*) override {
        try {
            py::gil_scoped_acquire a;
            if (auto override = pybind11::get_override(static_cast<const PluginInstanceBase*>(this), "initialize"))
                override();
        } catch (const std::exception &e) {
            CRIT << e.what();
        }
    }

    // Drop param
    void finalize(ExtensionRegistry*) override {
        try {
            py::gil_scoped_acquire a;
            if (auto override = pybind11::get_override(static_cast<const PluginInstanceBase*>(this), "finalize"))
                override();
        } catch (const std::exception &e) {
            CRIT << e.what();
        }
    }

    // Return extensions if no override
    vector<Extension*> extensions() override {
        return extensions_;
    }

//    QWidget *buildConfigWidget()
//    { PYBIND11_OVERRIDE(vector<Action>, PluginInstanceBase, buildConfigWidget, ); }

    py::object pathlibCachePath() const {
        py::gil_scoped_acquire a;
        return Path(PluginInstance::cacheDir()->path());
    }

    py::object pathlibConfigPath() const {
        py::gil_scoped_acquire a;
        return Path(PluginInstance::configDir()->path());
    }

    py::object pathlibDataPath() const {
        py::gil_scoped_acquire a;
        return Path(PluginInstance::dataDir()->path());
    }

private:
    py::object Path;
    vector<Extension*> extensions_;
};


template <class Base = Extension>
class PyExtensionTrampoline : public Base
{
public:
    using Base::Base;  // Inherit the constructors, pybind requirement

    PyExtensionTrampoline(const QString &id,
                          const QString &name,
                          const QString &description):
        id_(id),
        name_(name),
        description_(description)
    {}

    QString id() const override { return id_; }
    QString name() const override { return name_; }
    QString description() const override { return description_; }

protected:
    const QString id_;
    const QString name_;
    const QString description_;
};


template <class Base = FallbackHandler>
class PyFallbackHandlerTrampoline : public PyExtensionTrampoline<Base> {
public:
    using PyExtensionTrampoline<Base>::PyExtensionTrampoline; // Inherit constructors

    vector<shared_ptr<Item>> fallbacks(const QString &query) const override {
        try { PYBIND11_OVERRIDE_PURE(vector<shared_ptr<Item>>, Base, fallbacks, query); }
        catch (const std::exception &e) { CRIT << e.what(); return {}; }
    }

};


template <class Base = TriggerQueryHandler>
class PyTriggerQueryHandlerTrampoline : public PyExtensionTrampoline<Base> {
public:
    using PyExtensionTrampoline<Base>::PyExtensionTrampoline; // Inherit constructors

    PyTriggerQueryHandlerTrampoline(const QString &id,
                                    const QString &name,
                                    const QString &description,
                                    const QString &synopsis,
                                    const QString &defaultTrigger,
                                    const bool &allowTriggerRemap,
                                    const bool &supportsFuzzyMatching):
        PyExtensionTrampoline<Base>(id, name, description),
        synopsis_(synopsis),
        defaultTrigger_(defaultTrigger.isNull() ? QString("%1 ").arg(id) : defaultTrigger),
        allowTriggerRemap_(allowTriggerRemap),
        supportsFuzzyMatching_(supportsFuzzyMatching)
    {}

    QString synopsis() const override { return synopsis_; }
    QString defaultTrigger() const override { return defaultTrigger_; }
    bool allowTriggerRemap() const override { return allowTriggerRemap_; }
    bool supportsFuzzyMatching() const override { return supportsFuzzyMatching_; }

    bool fuzzyMatching() const override {
        try { PYBIND11_OVERRIDE(bool, Base, fuzzyMatching, ); }
        catch (const std::exception &e) { CRIT << e.what(); return Base::fuzzyMatching(); }
    }

    void setFuzzyMatching(bool enabled) override {
        try { PYBIND11_OVERRIDE(void, Base, setFuzzyMatching, enabled); }
        catch (const std::exception &e) { CRIT << e.what(); }
    }

    void handleTriggerQuery(TriggerQueryHandler::TriggerQuery *query) const override {
        try {PYBIND11_OVERRIDE_PURE(void, Base, handleTriggerQuery, query); }
        catch (const std::exception &e) { CRIT << e.what(); }
    }

protected:
    const QString synopsis_;
    const QString defaultTrigger_;
    const bool allowTriggerRemap_;
    const bool supportsFuzzyMatching_;
};


template <class Base = GlobalQueryHandler>
class PyGlobalQueryHandlerTrampoline : public PyTriggerQueryHandlerTrampoline<Base> {
public:
    using PyTriggerQueryHandlerTrampoline<Base>::PyTriggerQueryHandlerTrampoline; // Inherit constructors

    void handleTriggerQuery(TriggerQueryHandler::TriggerQuery *query) const override {
        try { PYBIND11_OVERRIDE(void, Base, handleTriggerQuery, query); }
        catch (const std::exception &e) { CRIT << e.what(); }
    }

    vector<RankItem> handleGlobalQuery(const GlobalQueryHandler::GlobalQuery *query) const override {
        try { PYBIND11_OVERRIDE_PURE(vector<RankItem>, Base, handleGlobalQuery, query); }
        catch (const std::exception &e) { CRIT << e.what(); return {}; }
    }
};


template <class Base = IndexQueryHandler>
class PyIndexQueryHandlerTrampoline : public PyGlobalQueryHandlerTrampoline<Base> {
public:

    PyIndexQueryHandlerTrampoline(const QString &id,
                        const QString &name,
                        const QString &description,
                        const QString &synopsis,
                        const QString &defaultTrigger,
                        const bool &allowTriggerRemap):
        PyGlobalQueryHandlerTrampoline<Base>(id,
                                   name,
                                   description,
                                   synopsis,
                                   defaultTrigger,
                                   allowTriggerRemap,
                                   true)
    {}

    vector<RankItem> handleGlobalQuery(const GlobalQueryHandler::GlobalQuery *query) const override {
        try { PYBIND11_OVERRIDE(vector<RankItem>, Base, handleGlobalQuery, query); }
        catch (const std::exception &e) { CRIT << e.what(); return {}; }
    }

    void updateIndexItems() override {
        try { PYBIND11_OVERRIDE_PURE(void, Base, updateIndexItems, ); }
        catch (const std::exception &e) { CRIT << e.what(); }
    }

    bool fuzzyMatching() const override
    { return IndexQueryHandler::fuzzyMatching(); }

    void setFuzzyMatching(bool enabled) override
    { IndexQueryHandler::setFuzzyMatching(enabled); }

};


//using TQHPI = ExtensionPluginInstance<TriggerQueryHandler>;
//using TQHPITB = PyTriggerQueryHandlerTrampoline<PyPluginInstanceTrampoline<TQHPI>>;
//class TQHPIT : TQHPITB
//{
//public:
//    TQHPIT(const QString &synopsis,
//           const QString &defaultTrigger,
//           const bool &allowTriggerRemap,
//           const bool &supportsFuzzyMatching):
//        TQHPITB(id(),
//                name(),
//                description(),
//                synopsis,
//                defaultTrigger,
//                allowTriggerRemap,
//                supportsFuzzyMatching)
//    {}
//};




















////QWidget *PyPluginInstance::buildConfigWidget()
////{
////    py::gil_scoped_acquire acquire;
////    static const char * ATTR_BUILD_CONFIG_WIDGET = "buildConfigWidget";
////    if (py::hasattr(*pyinst_, ATTR_BUILD_CONFIG_WIDGET))
////        if (auto func = pyinst_->attr(ATTR_BUILD_CONFIG_WIDGET); py::isinstance<py::function>(func)){
////            py::object pyWidgetObj = pyinst_->attr(ATTR_BUILD_CONFIG_WIDGET)();
////            py::object cppWidgetObj = py::reinterpret_borrow<py::object>(pyWidgetObj);
////            auto *w = py::cast<QWidget*>(cppWidgetObj);
////            return w;
////        }
////    return nullptr;
////}
///
///
