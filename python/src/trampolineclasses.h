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
#include <QCheckBox>
#include <QComboBox>
#include <QSettings>
#include <QDir>
#include <QDoubleSpinBox>
#include <QFormLayout>
#include <QLineEdit>
#include <QWidget>
using namespace albert;
using namespace std;


class PyItemTrampoline : Item
{
public:
    QString id() const override {
        try { PYBIND11_OVERRIDE_PURE(QString, Item, id, ); }
        catch (const std::exception &e) { CRIT << e.what(); return {}; }
    }

    QString text() const override {
        try { PYBIND11_OVERRIDE_PURE(QString, Item, text, ); }
        catch (const std::exception &e) { CRIT << e.what(); return {}; }
    }

    QString subtext() const override {
        try { PYBIND11_OVERRIDE_PURE(QString, Item, subtext, ); }
        catch (const std::exception &e) { CRIT << e.what(); return {}; }
    }

    QStringList iconUrls() const override {
        try { PYBIND11_OVERRIDE_PURE(QStringList, Item, iconUrls, ); }
        catch (const std::exception &e) { CRIT << e.what(); return {}; }
    }

    QString inputActionText() const override     {
        try { PYBIND11_OVERRIDE_PURE(QString, Item, inputActionText, ); }
        catch (const std::exception &e) { CRIT << e.what(); return {}; }
    }

    vector<Action> actions() const override {
        try { PYBIND11_OVERRIDE_PURE(vector<Action>, Item, actions, ); }
        catch (const std::exception &e) { CRIT << e.what(); return {}; }
    }
};


template <class PluginInstanceBase = PluginInstance>
class PyPluginInstanceTrampoline : public PluginInstanceBase
{
public:
    PyPluginInstanceTrampoline(vector<Extension*> e = {}) : extensions_(e) {
        py::gil_scoped_acquire a;
        Path = py::module_::import("pathlib").attr("Path");
    }

    vector<Extension*> extensions() override { return extensions_; }

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

    // Dynamically create widget
    QWidget *buildConfigWidget() override {
        auto *w = new QWidget;
        auto *l = new QFormLayout(w);
        w->setLayout(l);

        try {
            py::gil_scoped_acquire a;
            if (auto override = pybind11::get_override(static_cast<const PluginInstanceBase*>(this), "configWidget")){
                for (auto item : py::list(override())) {
                    auto dict = py::cast<py::dict>(item);
                    auto property_name = dict["property"].cast<QString>();
                    auto display_name = dict["label"].cast<QString>();
                    static const char *widget_properties = "widget_properties";

                    if (auto type = dict["type"].cast<QString>(); type == QStringLiteral("lineedit")){
                        auto *fw = new QLineEdit(w);
                        fw->setText(py::getattr(py::cast(this), py::cast(property_name)).template cast<QString>());
                        if (dict.contains(widget_properties))
                            applyWidgetProperties(fw, dict[widget_properties].cast<py::dict>());
                        l->addRow(display_name, fw);
                        QObject::connect(fw, &QLineEdit::editingFinished, fw, [this, fw, property_name](){
                            py::gil_scoped_acquire aq;
                            try { py::setattr(py::cast(this), py::cast(property_name), py::cast(fw->text())); }
                            catch (const std::exception &e) { CRIT << e.what(); }

                        });

                    } else if (type == QStringLiteral("checkbox")){
                        auto *fw = new QCheckBox(w);
                        fw->setChecked(py::getattr(py::cast(this), py::cast(property_name)).template cast<bool>());
                        if (dict.contains(widget_properties))
                            applyWidgetProperties(fw, dict[widget_properties].cast<py::dict>());
                        l->addRow(display_name, fw);
                        QObject::connect(fw, &QCheckBox::toggled, fw, [this, property_name](bool checked){
                            py::gil_scoped_acquire aq;
                            try { py::setattr(py::cast(this), py::cast(property_name), py::cast(checked)); }
                            catch (const std::exception &e) { CRIT << e.what(); }
                        });

                    } else if (type == QStringLiteral("combobox")){
                        auto *fw = new QComboBox(w);
                        auto text = py::getattr(py::cast(this), py::cast(property_name)).template cast<QString>();
                        for (auto &value : dict["items"].cast<py::list>()){
                            fw->addItem(value.cast<QString>());
                        }
                        fw->setCurrentText(text);
                        if (dict.contains(widget_properties))
                            applyWidgetProperties(fw, dict[widget_properties].cast<py::dict>());
                        l->addRow(display_name, fw);
                        QObject::connect(fw, &QComboBox::currentIndexChanged, fw, [this, cb=fw, property_name](){
                            auto current_text = cb->currentText();
                            py::gil_scoped_acquire aq;
                            try { py::setattr(py::cast(this), py::cast(property_name), py::cast(current_text)); }
                            catch (const std::exception &e) { CRIT << e.what(); }
                        });

                    } else if (type == QStringLiteral("spinbox")){
                        auto *fw = new QSpinBox(w);
                        fw->setValue(py::getattr(py::cast(this), py::cast(property_name)).template cast<int>());
                        if (dict.contains(widget_properties))
                            applyWidgetProperties(fw, dict[widget_properties].cast<py::dict>());
                        l->addRow(display_name, fw);
                        QObject::connect(fw, &QSpinBox::valueChanged, fw, [this, property_name](int value){
                            py::gil_scoped_acquire aq;
                            try { py::setattr(py::cast(this), py::cast(property_name), py::cast(value)); }
                            catch (const std::exception &e) { CRIT << e.what(); }
                        });

                    } else if (type == QStringLiteral("doublespinbox")){
                        auto *fw = new QDoubleSpinBox(w);
                        fw->setValue(py::getattr(py::cast(this), py::cast(property_name)).template cast<double>());
                        if (dict.contains(widget_properties))
                            applyWidgetProperties(fw, dict[widget_properties].cast<py::dict>());
                        l->addRow(display_name, fw);
                        QObject::connect(fw, &QDoubleSpinBox::valueChanged, fw, [this, property_name](double value){
                            py::gil_scoped_acquire aq;
                            try { py::setattr(py::cast(this), py::cast(property_name), py::cast(value)); }
                            catch (const std::exception &e) { CRIT << e.what(); }
                        });

                    } else
                        throw runtime_error(QString("Invalid config widget form layout row widget type: %1").arg(type).toStdString());
                }
                return w;
            }
        } catch (const std::exception &e) {
            CRIT << e.what();
            delete w;
        }
        return nullptr;
    }

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

    void writeConfig(QString key, const py::object &value) const {
        py::gil_scoped_acquire a;
        auto s = this->settings();

        if (py::isinstance<py::str>(value))
            s->setValue(key, value.cast<QString>());

        else if (py::isinstance<py::bool_>(value))
            s->setValue(key, value.cast<bool>());

        else if (py::isinstance<py::int_>(value))
            s->setValue(key, value.cast<int>());

        else if (py::isinstance<py::float_>(value))
            s->setValue(key, value.cast<double>());

        else
            WARN << "Invalid data type to write to settings. Has to be one of bool|int|float|str.";
    }

    py::object readConfig(QString key, const py::object &type) const {
        py::gil_scoped_acquire a;
        QVariant var = this->settings()->value(key);

        if (var.isNull())
            return py::none();

        if (type.attr("__name__").cast<QString>() == QStringLiteral("str"))
            return py::cast(var.toString());

        else if (type.attr("__name__").cast<QString>() == QStringLiteral("bool"))
            return py::cast(var.toBool());

        else if (type.attr("__name__").cast<QString>() == QStringLiteral("int"))
            return py::cast(var.toInt());

        else if (type.attr("__name__").cast<QString>() == QStringLiteral("float"))
            return py::cast(var.toDouble());

        else
            WARN << "Invalid data type to read from settings. Has to be one of bool|int|float|str.";

        return py::none();
    }

    static void applyWidgetProperties(QWidget *widget, py::dict widget_properties){
        py::gil_scoped_acquire a;
        for (auto &[k, v] : widget_properties) {
            std::string property_name = py::cast<string>(k);

            if (py::isinstance<py::bool_>(v))
                widget->setProperty(property_name.c_str(), py::cast<bool>(v));

            else if (py::isinstance<py::int_>(v))
                widget->setProperty(property_name.c_str(), py::cast<int>(v));

            else if (py::isinstance<py::float_>(v))
                widget->setProperty(property_name.c_str(), py::cast<double>(v));

            else if (py::isinstance<py::str>(v))
                widget->setProperty(property_name.c_str(), py::cast<QString>(v));

            else
                WARN << "Invalid data type set as widget property. Has to be one of bool|int|float|str.";
        }
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




















//QWidget *PyPluginInstance::buildConfigWidget()
//{
//    py::gil_scoped_acquire acquire;
//    static const char * ATTR_BUILD_CONFIG_WIDGET = "buildConfigWidget";
//    if (py::hasattr(*pyinst_, ATTR_BUILD_CONFIG_WIDGET))
//        if (auto func = pyinst_->attr(ATTR_BUILD_CONFIG_WIDGET); py::isinstance<py::function>(func)){
//            py::object pyWidgetObj = pyinst_->attr(ATTR_BUILD_CONFIG_WIDGET)();
//            py::object cppWidgetObj = py::reinterpret_borrow<py::object>(pyWidgetObj);
//            auto *w = py::cast<QWidget*>(cppWidgetObj);
//            return w;
//        }
//    return nullptr;
//}

//py::dict pyDict = py::getattr(py::cast(this), "__dict__");
//for (const auto& item : pyDict)
//    std::cout << py::str(item.first) << ": " << py::str(item.second) << std::endl;
