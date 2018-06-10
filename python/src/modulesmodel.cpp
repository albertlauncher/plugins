// Copyright (c) 2017 Manuel Schneider

#include "pythonmodulev1.h"
#include <QIcon>
#include <QStandardPaths>
#include <QFile>
#include <QDir>
#include "modulesmodel.h"
#include "extension.h"


namespace {
enum class Section{Name, Trigger, Version, Author, Count};
}


/** ***************************************************************************/
Python::ModulesModel::ModulesModel(Extension *extension, QObject *parent)
    : QAbstractTableModel(parent), extension(extension) {

    connect(extension, &Extension::modulesChanged,
            this, &ModulesModel::endResetModel);

}


/** ***************************************************************************/
int Python::ModulesModel::rowCount(const QModelIndex &) const {
    return static_cast<int>(extension->modules().size());
}



/** ***************************************************************************/
int Python::ModulesModel::columnCount(const QModelIndex &) const {
    return static_cast<int>(Section::Count);
}



/** ***************************************************************************/
QVariant Python::ModulesModel::headerData(int section, Qt::Orientation orientation, int role) const {
    // No sanity check necessary since
    if ( section<0 || static_cast<int>(Section::Count)<=section )
        return QVariant();


    if (orientation == Qt::Horizontal){
        switch (static_cast<Section>(section)) {
        case Section::Name:{
            switch (role) {
            case Qt::DisplayRole: return "Name";
            case Qt::ToolTipRole: return "The name of the module.";
            default: return QVariant();
            }
        }
        case Section::Trigger:{
            switch (role) {
            case Qt::DisplayRole: return "Trigger";
            case Qt::ToolTipRole: return "The term that triggers this extension.";
            default: return QVariant();
            }
        }
        case Section::Version:{
            switch (role) {
            case Qt::DisplayRole: return "Version";
            case Qt::ToolTipRole: return "The version of this extension.";
            default: return QVariant();
            }
        }
        case Section::Author:{
            switch (role) {
            case Qt::DisplayRole: return "Author";
            case Qt::ToolTipRole: return "The author of this extension.";
            default: return QVariant();
            }
        }
        default: return QVariant();
        }
    }
    return QVariant();
}



/** ***************************************************************************/
QVariant Python::ModulesModel::data(const QModelIndex &index, int role) const {
    if (!index.isValid()
            || index.row() >= static_cast<int>(extension->modules().size())
            || index.column() >= static_cast<int>(static_cast<int>(Section::Count)))
        return QVariant();

    switch (role) {
    case Qt::DecorationRole: {
        if ( static_cast<Section>(index.column()) == Section::Name ) {
            switch (extension->modules()[static_cast<size_t>(index.row())]->state()) {
            case PythonModuleV1::State::Unloaded:
                return QIcon(":plugin_notloaded");
            case PythonModuleV1::State::Loaded:
                return QIcon(":plugin_loaded");
            case PythonModuleV1::State::Error:
                return QIcon(":plugin_error");
            }
        }
        return QVariant();
    }
    case Qt::DisplayRole: {
        const PythonModuleV1 *module = extension->modules()[static_cast<size_t>(index.row())].get();
        switch (static_cast<Section>(index.column())) {
        case Section::Name:  return module->name();
        case Section::Trigger:  return module->trigger();
        case Section::Version:  return module->version();
        case Section::Author:  return module->author();
        default: return QVariant();
        }
    }
    case Qt::ToolTipRole: {
        QString toolTip;
        const PythonModuleV1 *module = extension->modules()[static_cast<size_t>(index.row())].get();

        toolTip = QString("<p><b>Name:</b> %1</p>").arg(module->name().toHtmlEscaped());

//        if (!module->version().isEmpty())
//            toolTip.append(QString("\nVersion: %1").arg(module->version()));

//        if (!module->author().isEmpty())
//            toolTip.append(QString("\nAuthor: %1").arg(module->author()));

        if (!module->dependencies().empty())
            toolTip.append(QString("<p><b>Dependencies:</b> %1</p>").arg(module->dependencies().join(", ").toHtmlEscaped()));

        toolTip.append(QString("<p><b>Path:</b>%1</p>").arg(module->path().toHtmlEscaped()));

        if (!module->description().isEmpty())
            toolTip.append(QString("<p><b>Description:</b> %1</p>").arg(QString(module->description().toHtmlEscaped()).replace("\n","<br>")));

        if (module->state() == PythonModuleV1::State::Error && !module->errorString().isEmpty())
            toolTip.append(QString("<p><font color=red><b>ERROR:</b>%1</font></p>").arg(QString(module->errorString().toHtmlEscaped()).replace("\n","<br>")));

        return QString("<html><head/><body>%1</body></html>").arg(toolTip);
    }
    case Qt::TextAlignmentRole:{
        switch (static_cast<Section>(index.column())) {
        case Section::Trigger:
        case Section::Version: return Qt::AlignCenter; break;
        case Section::Name:
        case Section::Author:
        default: return QVariant();
        }
    }
    case Qt::CheckStateRole:
        if ( static_cast<Section>(index.column()) == Section::Name )
            return (extension->isEnabled(*extension->modules()[static_cast<size_t>(index.row())].get()))
                    ? Qt::Checked : Qt::Unchecked;
    }
    return QVariant();
}


/** ***************************************************************************/
bool Python::ModulesModel::setData(const QModelIndex &index, const QVariant &value, int role) {
    if (!index.isValid() || index.row() < 0 || rowCount() <= index.row())
        return false;

    switch (role) {
    case Qt::CheckStateRole:
        if ( static_cast<Section>(index.column()) == Section::Name ) {
            extension->setEnabled(*extension->modules()[static_cast<size_t>(index.row())].get(),
                                  value == Qt::Checked);
            dataChanged(this->index(index.row(),0),
                        this->index(index.row(),static_cast<int>(Section::Count)));
            return true;
        }
    }
    return false;
}


/** ***************************************************************************/
Qt::ItemFlags Python::ModulesModel::flags(const QModelIndex &index) const {
    if (!index.isValid() || index.row() >= static_cast<int>(extension->modules().size()) )
        return Qt::NoItemFlags;
    return Qt::ItemIsEnabled | Qt::ItemIsUserCheckable | Qt::ItemNeverHasChildren;
}

