// Copyright (C) 2014-2018 Manuel Schneider

#include <QIcon>
#include <QStandardPaths>
#include <QFile>
#include <QDir>
#include <QDesktopServices>
#include <QUrl>
#include "externalextensionmodel.h"
#include "externalextension.h"


namespace {
enum class Section{Name, Trigger, Path, Count};
}


/** ***************************************************************************/
int ExternalExtensions::ExternalExtensionsModel::rowCount(const QModelIndex &) const {
    return static_cast<int>(externalExtensions_.size());
}



/** ***************************************************************************/
int ExternalExtensions::ExternalExtensionsModel::columnCount(const QModelIndex &) const {
    return static_cast<int>(Section::Count);
}



/** ***************************************************************************/
QVariant ExternalExtensions::ExternalExtensionsModel::headerData(int section, Qt::Orientation orientation, int role) const {
    // No sanity check necessary since
    if ( section<0 || static_cast<int>(Section::Count)<=section )
        return QVariant();


    if (orientation == Qt::Horizontal){
        switch (static_cast<Section>(section)) {
        case Section::Name:{
            switch (role) {
            case Qt::DisplayRole: return "Name";
            case Qt::ToolTipRole: return "The name of the extension.";
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
        case Section::Path:{
            switch (role) {
            case Qt::DisplayRole: return "Path";
            case Qt::ToolTipRole: return "The path of this extension.";
            default: return QVariant();
            }

        }
        default: return QVariant();
        }
    }
    return QVariant();
}



/** ***************************************************************************/
QVariant ExternalExtensions::ExternalExtensionsModel::data(const QModelIndex &index, int role) const {
    if (!index.isValid()
            || index.row() >= static_cast<int>(externalExtensions_.size())
            || index.column() >= static_cast<int>(static_cast<int>(Section::Count)))
        return QVariant();

    switch (role) {
    case Qt::DecorationRole: {
        if ( static_cast<Section>(index.column()) == Section::Name ) {
            switch (externalExtensions_[static_cast<size_t>(index.row())]->state()) {
            case ExternalExtension::State::Initialized:
                return QIcon(":plugin_loaded");
            case ExternalExtension::State::Error:
                return QIcon(":plugin_error");
            }
        }
        return QVariant();
    }
    case Qt::DisplayRole: {
        switch (static_cast<Section>(index.column())) {
        case Section::Name:  return externalExtensions_[static_cast<size_t>(index.row())]->name();
        case Section::Trigger:  return externalExtensions_[static_cast<size_t>(index.row())]->trigger();
        case Section::Path:  return externalExtensions_[static_cast<size_t>(index.row())]->path();
        default: return QVariant();
        }
    }
    case Qt::EditRole: {
        switch (static_cast<Section>(index.column())) {
        case Section::Name:  return externalExtensions_[static_cast<size_t>(index.row())]->name();
        case Section::Trigger:  return externalExtensions_[static_cast<size_t>(index.row())]->trigger();
        case Section::Path:  return externalExtensions_[static_cast<size_t>(index.row())]->path();
        default: return QVariant();
        }
    }
    case Qt::ToolTipRole: {
        QString toolTip;
        const ExternalExtension *ext = externalExtensions_[static_cast<size_t>(index.row())].get();

        toolTip = QString("ID: %1").arg(ext->id());

        if (!ext->version().isEmpty())
            toolTip.append(QString("\nVersion: %1").arg(ext->version()));

        if (!ext->author().isEmpty())
            toolTip.append(QString("\nAuthor: %1").arg(ext->author()));

        if (!ext->dependencies().empty())
            toolTip.append(QString("\nDependencies: %1").arg(ext->dependencies().join(", ")));

        toolTip.append(QString("\nPath: %1").arg(ext->path()));

        if (!ext->description().isEmpty())
            toolTip.append(QString("\nDescription: %1").arg(ext->description()));

        if (!ext->usageExample().isEmpty())
            toolTip.append(QString("\nUsage example: '%1'").arg(ext->usageExample()));

        if (ext->state() == ExternalExtension::State::Error && !ext->errorString().isEmpty())
            toolTip.append(QString("\nERROR: %1").arg(ext->errorString()));

        return toolTip;
    }
    default:
        return QVariant();
    }
}


/** ***************************************************************************/
Qt::ItemFlags ExternalExtensions::ExternalExtensionsModel::flags(const QModelIndex &index) const {
    if ( !index.isValid() )
        return Qt::NoItemFlags;
    return Qt::ItemIsSelectable|Qt::ItemIsEnabled;
}


/** ***************************************************************************/
void ExternalExtensions::ExternalExtensionsModel::onActivated(const QModelIndex &index) {
    QDesktopServices::openUrl(QUrl(externalExtensions_[static_cast<size_t>(index.row())]->path()));
}
