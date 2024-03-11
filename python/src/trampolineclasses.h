// Copyright (c) 2017-2023 Manuel Schneider

#pragma once

#include "cast_specialization.h" // Has to be imported first

#include <QCheckBox>
#include <QComboBox>
#include <QDir>
#include <QDoubleSpinBox>
#include <QFormLayout>
#include <QLabel>
#include <QLineEdit>
#include <QSettings>
#include <QString>
#include <QWidget>
#include <albert/extensionregistry.h>
#include <albert/fallbackhandler.h>
#include <albert/indexqueryhandler.h>
#include <albert/logging.h>
#include <albert/plugininstance.h>
#include <albert/pluginloader.h>
#include <albert/pluginmetadata.h>
using namespace albert;
using namespace std;


#define CATCH_PYBIND11_OVERRIDE_PURE(ret, base, func, ...) \
try { PYBIND11_OVERRIDE_PURE(ret, base, func, __VA_ARGS__ ); } \
catch (const std::exception &e) { CRIT << typeid(base).name() << #func << e.what(); }

#define CATCH_PYBIND11_OVERRIDE(ret, base, func, ...) \
try { PYBIND11_OVERRIDE(ret, base, func, __VA_ARGS__ ); } \
catch (const std::exception &e) { CRIT << typeid(base).name() << #func << e.what(); }


class PyPI : public PluginInstance
{
public:

    PyPI(const vector<Extension*> &e = {})
        : additionalExtensions_(e.begin(), e.end())
    {
        if (!e.empty())
            WARN << loader().metaData().id
                 << "Using the 'extensions' argument in the constructor is deprecated and will be "
                    "dropped in interface v3.0. Use the registerExtension and deregisterExtension "
                    "methods instead.";
    }

    /// Lock GIL before!
    /// Remove in 3.0
    void backwardCompatibileInit()
    {
        // To not break compat with 2.x. register ctor-passed extensions.
        // Exclude the root extension which will be autoloaded by the plugin loader

        if (auto self = py::cast(this); py::isinstance<Extension>(self))
        {
            auto *root_extension = self.cast<Extension*>();
            additionalExtensions_.erase(root_extension);
        }

        for (auto *e : additionalExtensions_)
            registry().registerExtension(e);
    }

    /// Lock GIL before!
    /// Remove in 3.0
    void backwardCompatibileFini()
    {
        for (auto *e : additionalExtensions_)
            registry().deregisterExtension(e);
    }

    void registerExtension(Extension *e) { registry().registerExtension(e); }

    void deregisterExtension(Extension *e) { registry().deregisterExtension(e); }

    py::object pathlibCachePath() const { return createPath(cacheLocation()); }

    py::object pathlibConfigPath() const { return createPath(configLocation()); }

    py::object pathlibDataPath() const { return createPath(dataLocation()); }

    void writeConfig(QString key, const py::object &value) const
    {
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

    py::object readConfig(QString key, const py::object &type) const
    {
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

private:

    py::object createPath(const QString &path) const
    {
        py::gil_scoped_acquire a;

        if (QDir dir(path); !dir.exists())
        {
            WARN << loader().metaData().id
                 << ": Implicit directory creation is a deprecated feature and will be dropped in "
                    "interace version 3.0!";

            if (!dir.mkpath("."))
                CRIT << "Failed to create path" << path;
        }

        return py::module_::import("pathlib").attr("Path")(path);  // should cache the module
    }

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

    // Todo: Remove with 3.0
    set<Extension*> additionalExtensions_;

};


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


template <class Base = Extension>
class PyE : public Base
{
public:
    // using Base::Base;  // Inherit the constructors, pybind requirement

    PyE(const QString &id, const QString &name, const QString &description)
        : id_(id), name_(name), description_(description)
    {
        // INFO << "PyE" << id_;
        // No way to cast to PI here to get the metadata
    }

    // template <typename T = Base, typename = std::enable_if_t<std::is_base_of<PluginInstance, T>::value>>
    // PyE():
    //     id_(PluginInstance::loader().metaData().id),
    //     name_(PluginInstance::loader().metaData().name),
    //     description_(PluginInstance::loader().metaData().description)
    // {
    //     // INFO << "PyE" << id_;
    // }

    QString id() const override { return id_; }
    QString name() const override { return name_; }
    QString description() const override { return description_; }

protected:

    const QString id_;
    const QString name_;
    const QString description_;

};


template <class Base = FallbackHandler>
class PyFQH : public PyE<Base>
{
public:

    using PyE<Base>::PyE; // Inherit constructors

    PyFQH(const QString &id, const QString &name, const QString &description)
        : PyE<Base>(id, name, description)
    {
        // INFO << "PyFQH" << id;
    }

    vector<shared_ptr<Item>> fallbacks(const QString &query) const override
    { CATCH_PYBIND11_OVERRIDE_PURE(vector<shared_ptr<Item>>, FallbackHandler, fallbacks, query); return {}; }

};


template <class Base = TriggerQueryHandler>
class PyTQH : public PyE<Base>
{
public:

    using PyE<Base>::PyE; // Inherit constructors

    PyTQH(const QString &id,
          const QString &name,
          const QString &description,
          const QString &synopsis,
          const QString &defaultTrigger,
          bool allowTriggerRemap,
          bool supportsFuzzyMatching):
        PyE<Base>(id, name, description),
        synopsis_(synopsis),
        defaultTrigger_(defaultTrigger.isEmpty() ? QString("%1 ").arg(id) : defaultTrigger),
        allowTriggerRemap_(allowTriggerRemap),
        supportsFuzzyMatching_(supportsFuzzyMatching)
    {
        // INFO << "PyTQH" << id;
    }

    PyTQH(const QString &synopsis,
          const QString &defaultTrigger,
          bool allowTriggerRemap,
          bool supportsFuzzyMatching):
        PyE<Base>(),
        synopsis_(synopsis),
        defaultTrigger_(defaultTrigger.isEmpty() ? QString("%1 ").arg(PyE<Base>::id_) : defaultTrigger),
        allowTriggerRemap_(allowTriggerRemap),
        supportsFuzzyMatching_(supportsFuzzyMatching)
    {
        // INFO << "PyTQH" << PyE<Base>::id_;
    }

    QString synopsis() const override
    { return synopsis_; }

    QString defaultTrigger() const override
    { return defaultTrigger_; }

    bool allowTriggerRemap() const override
    { return allowTriggerRemap_; }

    bool supportsFuzzyMatching() const override
    { return supportsFuzzyMatching_; }

    void setFuzzyMatching(bool enabled) override
    { CATCH_PYBIND11_OVERRIDE(void, Base, setFuzzyMatching, enabled); }

    void handleTriggerQuery(albert::Query *query) override
    { CATCH_PYBIND11_OVERRIDE_PURE(void, Base, handleTriggerQuery, query); }

protected:

    const QString synopsis_;
    const QString defaultTrigger_;
    const bool allowTriggerRemap_;
    const bool supportsFuzzyMatching_;

};


template <class Base = GlobalQueryHandler>
class PyGQH : public PyTQH<Base>
{
public:

    using PyTQH<Base>::PyTQH; // Inherit constructors

    PyGQH(const QString &id,
          const QString &name,
          const QString &description,
          const QString &synopsis,
          const QString &defaultTrigger,
          bool allowTriggerRemap,
          bool supportsFuzzyMatching):
        PyTQH<Base>(id, name, description, synopsis, defaultTrigger,
                    allowTriggerRemap, supportsFuzzyMatching)
    {
        // INFO << "PyGQH" << id;
    }

    // This is required due to the "final" quirks of the pybind trampoline chain
    // E < TQH < GQH < PyE < PyTQH < PyGQH
    //           ^(1)        ^(2)    ^(3)
    // (1) overrides handleTriggerQuery "final"
    // (2) overrides "pure" on python side
    // (3) has to override none pure otherwise calls will throw "call to pure" error
    void handleTriggerQuery(albert::Query *query) override
    { CATCH_PYBIND11_OVERRIDE(void, Base, handleTriggerQuery, query); }

    vector<RankItem> handleGlobalQuery(const albert::Query *query) const override
    { CATCH_PYBIND11_OVERRIDE_PURE(vector<RankItem>, Base, handleGlobalQuery, query); return {}; }

};


template <class Base = IndexQueryHandler>
class PyIQH : public PyGQH<Base>
{
public:

    using PyGQH<Base>::PyGQH; // Inherit constructors

    PyIQH(const QString &id, const QString &name, const QString &description,
          const QString &synopsis, const QString &defaultTrigger,
          bool allowTriggerRemap)
        : PyGQH<Base>(id, name, description, synopsis, defaultTrigger,
                      allowTriggerRemap, true)
    {
        // INFO << "PyIQH" << id;
    }

    PyIQH(const QString &synopsis, const QString &defaultTrigger, bool allowTriggerRemap)
        : PyGQH<Base>(synopsis, defaultTrigger, allowTriggerRemap, true)
    {
        // INFO << "PyIQH" << PyE<Base>::id_;
    }

    // This is required due to the "final" quirks of the pybind trampoline chain
    // E < TQH < GQH < IQH < PyE < PyTQH < PyGQH < PyIQH
    //                 ^(1)                ^(2)    ^(3)
    // (1) overrides handleGlobalQuery "final"
    // (2) overrides "pure" on python side
    // (3) has to override non-pure otherwise calls will throw "call to pure" error
    vector<RankItem> handleGlobalQuery(const Query *query) const override
    { CATCH_PYBIND11_OVERRIDE(vector<RankItem>, Base, handleGlobalQuery, query); return {}; }

    void updateIndexItems() override
    { CATCH_PYBIND11_OVERRIDE_PURE(void, Base, updateIndexItems, ); }

};
