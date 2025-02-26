// Copyright (c) 2017-2023 Manuel Schneider

#pragma once

#include "cast_specialization.hpp"  // Has to be imported first

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

// Workaround dysfunctional mixin behavior.
// See https://github.com/pybind/pybind11/issues/5405
#define WORKAROUND_PYBIND_5405(name) \
QString name() const override { \
    try { \
        py::gil_scoped_acquire gil; \
        if (auto py_instance = py::cast(this); py::isinstance<PluginInstance>(py_instance)) \
            return py::cast<PluginInstance*>(py_instance)->loader().metaData().name; \
        PYBIND11_OVERRIDE_PURE(QString, Base, name, ); \
    } \
    catch (const std::runtime_error &e) { \
        CRIT << __PRETTY_FUNCTION__ << e.what(); \
    } \
    return {}; \
}

class PyPI : public PluginInstance
{
public:

    vector<Extension *> extensions() override
    {
        py::gil_scoped_acquire gil;
        try
        {
            PYBIND11_OVERRIDE_PURE(vector<Extension *>, PluginInstance, extensions, );
        }
        catch (const std::exception &e)
        {
            if (auto py_instance = py::cast(this); py::isinstance<Extension>(py_instance))
                return {py::cast<Extension *>(py_instance)};

            pybind11::pybind11_fail(
                "Tried to call pure virtual function \""
                PYBIND11_STRINGIFY(Base) "::" "id" "\"");
        }

        return {};
    }

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
};


class PyItemTrampoline : Item
{
public:
    QString id() const override
    {
        CATCH_PYBIND11_OVERRIDE_PURE(QString, Item, id);
        return {};
    }

    QString text() const override
    {
        CATCH_PYBIND11_OVERRIDE_PURE(QString, Item, text);
        return {};
    }

    QString subtext() const override
    {
        CATCH_PYBIND11_OVERRIDE_PURE(QString, Item, subtext);
        return {};
    }

    QStringList iconUrls() const override
    {
        CATCH_PYBIND11_OVERRIDE_PURE(QStringList, Item, iconUrls);
        return {};
    }

    QString inputActionText() const override
    {
        CATCH_PYBIND11_OVERRIDE_PURE(QString, Item, inputActionText);
        return {};
    }

    vector<Action> actions() const override
    {
        CATCH_PYBIND11_OVERRIDE_PURE(vector<Action>, Item, actions);
        return {};
    }
};


template <class Base = Extension>
class PyE : public Base
{
public:
    WORKAROUND_PYBIND_5405(id)
    WORKAROUND_PYBIND_5405(name)
    WORKAROUND_PYBIND_5405(description)
};

template <class Base = TriggerQueryHandler>
class PyTQH : public PyE<Base>
{
public:
    QString synopsis(const QString &query) const override
    {
        CATCH_PYBIND11_OVERRIDE(QString, Base, synopsis, query);
        return {};
    }

    bool allowTriggerRemap() const override
    {
        CATCH_PYBIND11_OVERRIDE(bool, Base, allowTriggerRemap);
        return {};
    }

    QString defaultTrigger() const override
    {
        CATCH_PYBIND11_OVERRIDE(QString, Base, defaultTrigger);
        return {};
    }

    bool supportsFuzzyMatching() const override
    {
        CATCH_PYBIND11_OVERRIDE(bool, Base, supportsFuzzyMatching);
        return {};
    }

    void setFuzzyMatching(bool enabled) override
    {
        CATCH_PYBIND11_OVERRIDE(void, Base, setFuzzyMatching, enabled);
    }

    // No type mismatch workaround required since base class is not called.
    void handleTriggerQuery(albert::Query &query) override
    {
        albert::Query * query_ptr = &query;
        CATCH_PYBIND11_OVERRIDE_PURE(void, Base, handleTriggerQuery, query_ptr);
    }
};


template <class Base = GlobalQueryHandler>
class PyGQH : public PyTQH<Base>
{
public:
    // This is required due to the "final" quirks of the pybind trampoline chain
    // E < TQH < GQH < PyE < PyTQH < PyGQH
    //           ^(1)        ^(2)    ^(3)
    // (1) overrides handleTriggerQuery "final"
    // (2) overrides "pure" on python side
    // (3) has to override non-pure otherwise calls will throw "call to pure" error
    void handleTriggerQuery(albert::Query &query) override
    {
        // PyBind does not suport passing reference, but instead tries to copy. Workaround
        // converting to pointer. Needed because PYBIND11_OVERRIDE_PURE introduces type mismatch.
        albert::Query * query_ptr = &query;
        PYBIND11_OVERRIDE_IMPL(void, Base, "handleTriggerQuery", query_ptr);  // returns on success
        return Base::handleTriggerQuery(query);  // otherwise call base class
    }

    // No type mismatch workaround required since base class is not called.
    vector<RankItem> handleGlobalQuery(const albert::Query &query) override
    {
        CATCH_PYBIND11_OVERRIDE_PURE(vector<RankItem>, Base, handleGlobalQuery, &query);
        return {};
    }
};


template <class Base = IndexQueryHandler>
class PyIQH : public PyGQH<Base>
{
public:
    // This is required due to the "final" quirks of the pybind trampoline chain
    // E < TQH < GQH < IQH < PyE < PyTQH < PyGQH < PyIQH
    //                 ^(1)                ^(2)    ^(3)
    // (1) overrides handleGlobalQuery "final"
    // (2) overrides "pure" on python side
    // (3) has to override non-pure otherwise calls will throw "call to pure" error
    vector<RankItem> handleGlobalQuery(const albert::Query &query) override
    {
        // PyBind does not suport passing reference, but instead tries to copy. Workaround
        // converting to pointer. Needed because PYBIND11_OVERRIDE_PURE introduces type mismatch.
        const albert::Query * query_ptr = &query;
        PYBIND11_OVERRIDE_IMPL(vector<RankItem>, Base, "handleGlobalQuery", query_ptr);  // returns on success
        return Base::handleGlobalQuery(query);  // otherwise call base class
    }

    void updateIndexItems() override
    { CATCH_PYBIND11_OVERRIDE_PURE(void, Base, updateIndexItems); }
};


template <class Base = FallbackHandler>
class PyFQH : public PyE<Base>
{
public:
    vector<shared_ptr<Item>> fallbacks(const QString &query) const override
    {
        CATCH_PYBIND11_OVERRIDE_PURE(vector<shared_ptr<Item>>, FallbackHandler, fallbacks, query);
        return {};
    }
};
