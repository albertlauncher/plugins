// Copyright (c) 2017-2018 Manuel Schneider

#pragma once
#include <pybind11/embed.h>
#include <QString>
namespace py = pybind11;

//  Python string <-> QString conversion
namespace pybind11 {
namespace detail {

    template <> struct type_caster<QString> {
    PYBIND11_TYPE_CASTER(QString, _("QString"));
    private:
        using str_caster_t = make_caster<std::string>;
        str_caster_t str_caster;
    public:
        bool load(handle src, bool convert) {
            if (str_caster.load(src, convert)) { value = QString::fromStdString(str_caster); return true; }
            return false;
        }
        static handle cast(const QString &s, return_value_policy policy, handle parent) {
            return str_caster_t::cast(s.toStdString(), policy, parent);
        }
    };



    //template <> struct type_caster<QString> {
    //public:
    //    PYBIND11_TYPE_CASTER(QString, _("str"));
    //    bool load(handle src, bool) {
    //        PyObject *source = src.ptr();
    //        if (!PyUnicode_Check(source)) return false;
    //        value = PyUnicode_AsUTF8(source);
    //        return true;
    //    }
    //    static handle cast(QString src, return_value_policy /* policy */, handle /* parent */) {
    //        return PyUnicode_FromString(src.toUtf8().constData());
    //    }
    //};
}}
