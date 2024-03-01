// Copyright (c) 2017-2023 Manuel Schneider

#pragma once
#include "albert/extensionregistry.h"
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
#include <QLabel>
using namespace albert;
using namespace std;

#define CATCH_PYBIND11_OVERRIDE_PURE(ret, base, func, ...) \
    try { PYBIND11_OVERRIDE_PURE(ret, base, func, __VA_ARGS__ ); } \
    catch (const std::exception &e) { CRIT << e.what(); }

#define CATCH_PYBIND11_OVERRIDE(ret, base, func, ...) \
    try { PYBIND11_OVERRIDE(ret, base, func, __VA_ARGS__ ); } \
    catch (const std::exception &e) { CRIT << e.what(); }

class PyItemTrampoline : Item
{
public:
    QString id() const override
    { CATCH_PYBIND11_OVERRIDE_PURE(QString, Item, id); return {}; }

    QString text() const override
    { CATCH_PYBIND11_OVERRIDE_PURE(QString, Item, text); return {}; }

    QString subtext() const override
    { CATCH_PYBIND11_OVERRIDE_PURE(QString, Item, subtext, ); return {}; }

    QStringList iconUrls() const override
    { CATCH_PYBIND11_OVERRIDE_PURE(QStringList, Item, iconUrls, ); return {}; }

    QString inputActionText() const override
    { CATCH_PYBIND11_OVERRIDE_PURE(QString, Item, inputActionText, ); return {}; }

    vector<Action> actions() const override
    { CATCH_PYBIND11_OVERRIDE_PURE(vector<Action>, Item, actions, ); return {}; }
};

class PyPluginInstanceTrampoline : public PluginInstance
{
public:
    PyPluginInstanceTrampoline(const vector<Extension*> &e = {})
        : extensions_(e) {}

    // Drop param
    void initialize(ExtensionRegistry &registry, map<QString,PluginInstance*>) override
    {
        try {
            py::gil_scoped_acquire a;
            if (auto override = pybind11::get_override(static_cast<const PluginInstance*>(this), "initialize"))
                override();
        } catch (const std::exception &e) {
            CRIT << e.what();
        }

        // TODO: adopt api
        for (auto *e : extensions_)
            registry.registerExtension(e);
    }

    // Drop param
    void finalize(ExtensionRegistry &registry) override
    {
        // TODO: adopt api
        for (auto *e : extensions_)
            registry.deregisterExtension(e);

        try {
            py::gil_scoped_acquire a;
            if (auto override = pybind11::get_override(static_cast<const PluginInstance*>(this), "finalize"))
                override();
        } catch (const std::exception &e) {
            CRIT << e.what();
        }
    }

    // Dynamically create widget
    QWidget *buildConfigWidget() override
    {
        auto *w = new QWidget;
        auto *l = new QFormLayout(w);
        l->setContentsMargins(0,0,0,0);
        w->setLayout(l);

        static const char *key_label = "label";
        static const char *key_property = "property";
        static const char *key_text = "text";
        static const char *key_type = "type";
        static const char *key_items = "items";

        try
        {
            py::gil_scoped_acquire a;
            if (auto override = pybind11::get_override(static_cast<const PluginInstance*>(this), "configWidget"))
            {
                for (auto item : py::list(override()))
                {
                    auto row_spec = py::cast<py::dict>(item);

                    if (auto type = row_spec[key_type].cast<QString>(); type == QStringLiteral("lineedit"))
                    {
                        auto *fw = new QLineEdit(w);
                        auto property_name = row_spec[key_property].cast<QString>();

                        fw->setText(getattr<QString>(property_name));

                        QObject::connect(fw, &QLineEdit::editingFinished, fw, [this, fw, property_name](){
                            py::gil_scoped_acquire aq;
                            try { setattr(property_name, fw->text()); }
                            catch (const std::exception &e) { CRIT << e.what(); }
                        });

                        applyWidgetPropertiesIfAny(fw, row_spec);

                        l->addRow(row_spec[key_label].cast<QString>(), fw);
                    }
                    else if (type == QStringLiteral("checkbox"))
                    {
                        auto *fw = new QCheckBox(w);
                        auto property_name = row_spec[key_property].cast<QString>();

                        fw->setChecked(getattr<bool>(property_name));

                        QObject::connect(fw, &QCheckBox::toggled, fw, [this, property_name](bool checked){
                            py::gil_scoped_acquire aq;
                            try { setattr(property_name, checked); }
                            catch (const std::exception &e) { CRIT << e.what(); }
                        });

                        applyWidgetPropertiesIfAny(fw, row_spec);

                        l->addRow(row_spec[key_label].cast<QString>(), fw);
                    }
                    else if (type == QStringLiteral("combobox"))
                    {
                        auto *fw = new QComboBox(w);
                        auto property_name = row_spec[key_property].cast<QString>();

                        for (auto &value : row_spec[key_items].cast<py::list>())
                            fw->addItem(value.cast<QString>());

                        fw->setCurrentText(getattr<QString>(property_name));

                        QObject::connect(fw, &QComboBox::currentIndexChanged, fw, [this, cb=fw, property_name](){
                            py::gil_scoped_acquire aq;
                            try { setattr(property_name, cb->currentText()); }
                            catch (const std::exception &e) { CRIT << e.what(); }
                        });

                        applyWidgetPropertiesIfAny(fw, row_spec);

                        l->addRow(row_spec[key_label].cast<QString>(), fw);
                    }
                    else if (type == QStringLiteral("spinbox"))
                    {
                        auto *fw = new QSpinBox(w);
                        auto property_name = row_spec[key_property].cast<QString>();

                        fw->setValue(getattr<int>(property_name));

                        QObject::connect(fw, &QSpinBox::valueChanged, fw, [this, property_name](int value){
                            py::gil_scoped_acquire aq;
                            try { setattr(property_name, value); }
                            catch (const std::exception &e) { CRIT << e.what(); }
                        });

                        applyWidgetPropertiesIfAny(fw, row_spec);

                        l->addRow(row_spec[key_label].cast<QString>(), fw);
                    }
                    else if (type == QStringLiteral("doublespinbox"))
                    {
                        auto *fw = new QDoubleSpinBox(w);
                        auto property_name = row_spec[key_property].cast<QString>();

                        fw->setValue(getattr<double>(property_name));

                        QObject::connect(fw, &QDoubleSpinBox::valueChanged, fw, [this, property_name](double value){
                            py::gil_scoped_acquire aq;
                            try { setattr(property_name, value); }
                            catch (const std::exception &e) { CRIT << e.what(); }
                        });

                        applyWidgetPropertiesIfAny(fw, row_spec);

                        l->addRow(row_spec[key_label].cast<QString>(), fw);
                    }
                    else if (type == QStringLiteral("label"))
                    {
                        auto *lbl = new QLabel(w);
                        lbl->setText(row_spec[key_text].cast<QString>());
                        lbl->setWordWrap(true);
                        lbl->setOpenExternalLinks(true);
                        applyWidgetPropertiesIfAny(lbl, row_spec);
                        l->addRow(lbl);
                    }
                    else
                        throw runtime_error(QString("Invalid config widget form layout row widget type: %1").arg(type).toStdString());
                }
                return w;
            }
        }
        catch (const std::exception &e)
        {
            CRIT << e.what();
            delete w;
        }
        return nullptr;
    }

    py::object pathlibCachePath() const { return createPath(cacheDir().path()); }

    py::object pathlibConfigPath() const { return createPath(configDir().path()); }

    py::object pathlibDataPath() const { return createPath(dataDir().path()); }

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

private:

    /// Get a property of this Python instance
    /// DOES NOT LOCK THE GIL!
    template <class T>
    inline T getattr(QString property_name)
    { return py::getattr(py::cast(this), py::cast(property_name)).template cast<T>(); }

    /// Set a property of this Python instance
    /// DOES NOT LOCK THE GIL!
    template <class T>
    inline void setattr(QString property_name, T value)
    { return py::setattr(py::cast(this), py::cast(property_name), py::cast(value)); }

    static py::object createPath(const QString &path)
    {
        py::gil_scoped_acquire a;
        return py::module_::import("pathlib").attr("Path")(path); // should cache
    }

    static void applyWidgetPropertiesIfAny(QWidget *widget, py::dict spec)
    {
        static const char *key_widget_properties = "widget_properties";
        py::gil_scoped_acquire a;
        if (spec.contains(key_widget_properties))
        {
            for (auto &[k, v] : spec[key_widget_properties].cast<py::dict>())
            {
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
    }

protected:
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
class PyFallbackHandlerTrampoline : public PyExtensionTrampoline<Base>
{
public:
    using PyExtensionTrampoline<Base>::PyExtensionTrampoline; // Inherit constructors

    vector<shared_ptr<Item>> fallbacks(const QString &query) const override
    { CATCH_PYBIND11_OVERRIDE_PURE(vector<shared_ptr<Item>>, Base, fallbacks, query); return {}; }

};

template <class Base = TriggerQueryHandler>
class PyTriggerQueryHandlerTrampoline : public PyExtensionTrampoline<Base>
{
public:
    using PyExtensionTrampoline<Base>::PyExtensionTrampoline; // Inherit constructors

    PyTriggerQueryHandlerTrampoline(const QString &id,
                                    const QString &name,
                                    const QString &description,
                                    const QString &synopsis,
                                    const QString &defaultTrigger,
                                    bool allowTriggerRemap,
                                    bool supportsFuzzyMatching):
        PyExtensionTrampoline<Base>(id, name, description),
        synopsis_(synopsis),
        defaultTrigger_(defaultTrigger.isNull() ? QString("%1 ").arg(id) : defaultTrigger),
        allowTriggerRemap_(allowTriggerRemap),
        supportsFuzzyMatching_(supportsFuzzyMatching)
    {}

    QString synopsis() const override
    { return synopsis_; }

    QString defaultTrigger() const override
    { return defaultTrigger_; }

    bool allowTriggerRemap() const override
    { return allowTriggerRemap_; }

    bool supportsFuzzyMatching() const override
    { return supportsFuzzyMatching_; }

    bool fuzzyMatching() const override
    {
        CATCH_PYBIND11_OVERRIDE(bool, Base, fuzzyMatching);
        return Base::fuzzyMatching();
    }

    void setFuzzyMatching(bool enabled) override
    { CATCH_PYBIND11_OVERRIDE(void, Base, setFuzzyMatching, enabled); }

    void handleTriggerQuery(TriggerQueryHandler::TriggerQuery *query) const override
    { CATCH_PYBIND11_OVERRIDE_PURE(void, Base, handleTriggerQuery, query); }

protected:
    const QString synopsis_;
    const QString defaultTrigger_;
    const bool allowTriggerRemap_;
    const bool supportsFuzzyMatching_;
};

template <class Base = GlobalQueryHandler>
class PyGlobalQueryHandlerTrampoline : public PyTriggerQueryHandlerTrampoline<Base>
{
public:
    using PyTriggerQueryHandlerTrampoline<Base>::PyTriggerQueryHandlerTrampoline; // Inherit constructors

    void handleTriggerQuery(TriggerQueryHandler::TriggerQuery *query) const override
    { CATCH_PYBIND11_OVERRIDE(void, Base, handleTriggerQuery, query); }

    vector<RankItem> handleGlobalQuery(const GlobalQueryHandler::GlobalQuery *query) const override
    { CATCH_PYBIND11_OVERRIDE_PURE(vector<RankItem>, Base, handleGlobalQuery, query); return {}; }
};

template <class Base = IndexQueryHandler>
class PyIndexQueryHandlerTrampoline : public PyGlobalQueryHandlerTrampoline<Base>
{
public:

    using PyGlobalQueryHandlerTrampoline<Base>::PyGlobalQueryHandlerTrampoline; // Inherit constructors

    PyIndexQueryHandlerTrampoline(const QString &id,
                                  const QString &name,
                                  const QString &description,
                                  const QString &synopsis,
                                  const QString &defaultTrigger,
                                  bool allowTriggerRemap):
        PyGlobalQueryHandlerTrampoline<Base>(id,
                                             name,
                                             description,
                                             synopsis,
                                             defaultTrigger,
                                             allowTriggerRemap,
                                             true)
    {}

    vector<RankItem> handleGlobalQuery(const GlobalQueryHandler::GlobalQuery *query) const override
    { CATCH_PYBIND11_OVERRIDE(vector<RankItem>, Base, handleGlobalQuery, query); return {}; }

    void updateIndexItems() override
    { CATCH_PYBIND11_OVERRIDE_PURE(void, Base, updateIndexItems, ); }

    bool fuzzyMatching() const override
    { return IndexQueryHandler::fuzzyMatching(); }

    void setFuzzyMatching(bool enabled) override
    { IndexQueryHandler::setFuzzyMatching(enabled); }

};





// class TriggerQueryHandlerPlugin : public PluginInstance, public TriggerQueryHandler
// {
// public:
//     QString id() const override { return PluginInstance::id(); }
//     QString name() const override { return PluginInstance::name(); }
//     QString description() const override { return PluginInstance::description(); }
// };



// template <class Base = TriggerQueryHandler>
// class TriggerQueryHandlerPluginTrampoline : public PyPluginInstanceTrampoline, public PyTriggerQueryHandlerTrampoline<Base>
// {
// public:

//     TriggerQueryHandlerPluginTrampoline(const QString &synopsis,
//                                         const QString &defaultTrigger,
//                                         bool allowTriggerRemap,
//                                         bool supportsFuzzyMatching,
//                                         const vector<Extension*> &additionalExtensions):
//         PyPluginInstanceTrampoline(additionalExtensions),
//         PyTriggerQueryHandlerTrampoline<>(PluginInstance::id(),
//                                           PluginInstance::name(),
//                                           PluginInstance::description(),
//                                           synopsis,
//                                           defaultTrigger,
//                                           allowTriggerRemap,
//                                           supportsFuzzyMatching)
//     { extensions_.push_back(this); }
// };


















// class GlobalQueryHandlerPluginTrampoline : public PyPluginInstanceTrampoline, public PyGlobalQueryHandlerTrampoline<>
// {
// public:
//     GlobalQueryHandlerPluginTrampoline(const QString &synopsis,
//                                        const QString &defaultTrigger,
//                                        bool allowTriggerRemap,
//                                        bool supportsFuzzyMatching,
//                                        const vector<Extension*> &additionalExtensions):
//         PyPluginInstanceTrampoline(additionalExtensions),
//         PyGlobalQueryHandlerTrampoline<>(PluginInstance::id(),
//                                          PluginInstance::name(),
//                                          PluginInstance::description(),
//                                          synopsis,
//                                          defaultTrigger,
//                                          allowTriggerRemap,
//                                          supportsFuzzyMatching)
//     { extensions_.push_back(this); }
// };




// class IndexQueryHandlerPluginTrampoline : public PyPluginInstanceTrampoline, public PyIndexQueryHandlerTrampoline<>
// {
// public:
//     IndexQueryHandlerPluginTrampoline(const QString &synopsis,
//                                       const QString &defaultTrigger,
//                                       bool allowTriggerRemap,
//                                       const vector<Extension*> &additionalExtensions):
//         PyPluginInstanceTrampoline(additionalExtensions),
//         PyIndexQueryHandlerTrampoline<>(PluginInstance::id(),
//                                         PluginInstance::name(),
//                                         PluginInstance::description(),
//                                         synopsis,
//                                         defaultTrigger,
//                                         allowTriggerRemap)
//     { extensions_.push_back(this); }
// };




















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
