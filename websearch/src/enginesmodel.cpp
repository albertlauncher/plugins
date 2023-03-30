// Copyright (c) 2022-2023 Manuel Schneider


#include <QDir>
#include <QDebug>
#include <QFileInfo>
#include <QIcon>
#include <QMimeData>
#include <QStandardPaths>
#include <QUuid>
#include "enginesmodel.h"
#include "plugin.h"
#include "searchengine.h"

namespace {
enum class Section{ Name, Trigger, URL} ;
const int sectionCount = 3;
std::map<QString,QIcon> iconCache;
}


EnginesModel::EnginesModel(Plugin *extension, QObject *parent)
    : QAbstractTableModel(parent), extension_(extension)
{

}


int EnginesModel::rowCount(const QModelIndex &) const
{
    return static_cast<int>(extension_->engines().size());
}


int EnginesModel::columnCount(const QModelIndex &) const
{
    return sectionCount;
}


QVariant EnginesModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    // No sanity check necessary since
    if ( section < 0 || sectionCount <= section )
        return QVariant();


    if (orientation == Qt::Horizontal){
        switch (static_cast<Section>(section)) {
        case Section::Name:{
            switch (role) {
            case Qt::DisplayRole: return "Name";
            case Qt::ToolTipRole: return "The name of the searchengine.";
            default: return QVariant();
            }

        }
        case Section::Trigger:{
            switch (role) {
            case Qt::DisplayRole: return "Short";
            case Qt::ToolTipRole: return "The short name for this searchengine.";
            default: return QVariant();
            }

        }
        case Section::URL:{
            switch (role) {
            case Qt::DisplayRole: return "URL";
            case Qt::ToolTipRole: return "The URL of this searchengine. %s will be replaced by your searchterm.";
            default: return QVariant();
            }

        }
        }
    }
    return QVariant();
}


QVariant EnginesModel::data(const QModelIndex &index, int role) const
{
    if ( !index.isValid() ||
         index.row() >= static_cast<int>(extension_->engines().size()) ||
         index.column() >= sectionCount )
        return QVariant();

    switch (role) {
    case Qt::DisplayRole:
    case Qt::EditRole: {
        switch (static_cast<Section>(index.column())) {
        case Section::Name:
            return extension_->engines()[static_cast<ulong>(index.row())].name;
        case Section::Trigger:{
            auto trigger = extension_->engines()[static_cast<ulong>(index.row())].trigger;
            return trigger.replace(" ", "•");
        }
        case Section::URL:
            return extension_->engines()[static_cast<ulong>(index.row())].url;
        }
        break;
    }
    case Qt::DecorationRole: {
        if (static_cast<Section>(index.column()) == Section::Name) {
            // Resizing request thounsands of repaints. Creating an icon for
            // ever paint event is to expensive. Therefor maintain an icon cache
            const QString &iconPath = extension_->engines()[static_cast<ulong>(index.row())].iconPath;
            std::map<QString,QIcon>::iterator it = iconCache.find(iconPath);
            if ( it != iconCache.end() )
                return it->second;
            return iconCache.insert(std::make_pair(iconPath, QIcon(iconPath))).second;
        }
        break;
    }
    case Qt::ToolTipRole:
        return "Double click to edit. Drag and drop to reorder.";
    }
    return {};
}


bool EnginesModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if ( !index.isValid() ||
         index.row() >= static_cast<int>(extension_->engines().size()) ||
         index.column() >= sectionCount)
        return false;

    switch (role) {
    case Qt::DisplayRole: {
        if (!value.canConvert(QMetaType(QMetaType::QString)))
            return false;
        QString s = value.toString();
        switch (static_cast<Section>(index.column())) {
        case Section::Name: {
            std::vector<SearchEngine> newEngines = extension_->engines();
            newEngines[static_cast<ulong>(index.row())].name = s;
            extension_->setEngines(newEngines);
            emit dataChanged(index, index, QVector<int>({Qt::DisplayRole}));
            return true;
        }
        case Section::Trigger: {
            std::vector<SearchEngine> newEngines = extension_->engines();
            newEngines[static_cast<ulong>(index.row())].trigger = s;
            extension_->setEngines(newEngines);
            emit dataChanged(index, index, QVector<int>({Qt::DisplayRole}));
            return true;
        }
        case Section::URL: {
            std::vector<SearchEngine> newEngines = extension_->engines();
            newEngines[static_cast<ulong>(index.row())].url = s;
            extension_->setEngines(newEngines);
            emit dataChanged(index, index, QVector<int>({Qt::DisplayRole}));
            return true;
        }
        }
        return false;
    }
    case Qt::DecorationRole: {
        QFileInfo fileInfo(value.toString());

        if ( !fileInfo.exists() )
            return false;

        // Remove icon from cache
        iconCache.erase(extension_->engines()[static_cast<ulong>(index.row())].iconPath);

        // Create extension dir if necessary
        QDir configDir = QDir(QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation));
        if ( !configDir.exists(extension_->id()) ) {
            if ( !configDir.mkdir(extension_->id()) ) {
                qWarning() << "Could not create extension data dir.";
                return false;
            }
        }

        configDir.cd(extension_->id());

        // Build the new random path
        QString newFilePath = configDir.filePath(QString("%1.%2")
                                               .arg(QUuid::createUuid().toString())
                                               .arg(fileInfo.suffix()));

        // Copy the file into data dir
        if ( !QFile::copy(fileInfo.filePath(), newFilePath) ) {
            qWarning() << "Could not copy icon to cache.";
            return false;
        }

        // Remove old icon and set the copied file as icon
        std::vector<SearchEngine> newEngines = extension_->engines();
        QFile::remove(newEngines[static_cast<ulong>(index.row())].iconPath);
        newEngines[static_cast<ulong>(index.row())].iconPath = newFilePath;
        extension_->setEngines(newEngines);

        // Update the icon in the first section of the row
        QModelIndex firstSectionIndex = index.model()->index(index.row(), 0);
        dataChanged(firstSectionIndex, firstSectionIndex, QVector<int>({Qt::DecorationRole}));

        return true;
    }
    default:
        return false;
    }
}


Qt::ItemFlags EnginesModel::flags(const QModelIndex &index) const
{
    if (index.isValid())
        return QAbstractTableModel::flags(index) | Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsDragEnabled;
    else
        return QAbstractTableModel::flags(index) | Qt::ItemIsDropEnabled;
}


bool EnginesModel::insertRows(int position, int rows, const QModelIndex &)
{
    if ( position < 0 || rows < 1 ||
         static_cast<int>(extension_->engines().size()) < position)
        return false;

    beginInsertRows(QModelIndex(), position, position + rows - 1);
    std::vector<SearchEngine> newEngines = extension_->engines();
    for ( int row = position; row < position + rows; ++row )
        newEngines.insert(newEngines.begin() + row,
                          SearchEngine({"<name>", "<trigger>", ":default",
                                        "<http://url/containing/the/?query=%s>"}));
    extension_->setEngines(newEngines);
    endInsertRows();
    return true;
}


bool EnginesModel::removeRows(int position, int rows, const QModelIndex &)
{
    if ( position < 0 || rows < 1 ||
         static_cast<int>(extension_->engines().size()) < position + rows)
        return false;

    beginRemoveRows(QModelIndex(), position, position + rows - 1);
    std::vector<SearchEngine> newEngines = extension_->engines();
    newEngines.erase(newEngines.begin() + position,
                     newEngines.begin() + position + rows);
    extension_->setEngines(newEngines);
    endRemoveRows();
    return true;
}


bool EnginesModel::moveRows(const QModelIndex &srcParent, int srcRow, int cnt,
                            const QModelIndex &dstParent, int dstRow)
{
    if ( srcRow < 0 || cnt < 1 || dstRow < 0 ||
         static_cast<int>(extension_->engines().size()) < srcRow + cnt - 1 ||
         static_cast<int>(extension_->engines().size()) < dstRow ||
         ( srcRow <= dstRow && dstRow < srcRow + cnt) ) // If its inside the source do nothing
        return false;

    std::vector<SearchEngine> newEngines = extension_->engines();
    beginMoveRows(srcParent, srcRow, srcRow + cnt - 1, dstParent, dstRow);
    newEngines.insert(newEngines.begin() + dstRow,
                      extension_->engines().begin() + srcRow,
                      extension_->engines().begin() + srcRow + cnt);
    if ( srcRow < dstRow )
        newEngines.erase(newEngines.begin() + srcRow,
                         newEngines.begin() + srcRow + cnt);
    else
        newEngines.erase(newEngines.begin() + srcRow + cnt,
                         newEngines.begin() + srcRow + cnt * 2);
    extension_->setEngines(newEngines);
    endMoveRows();
    return true;
}


void EnginesModel::restoreDefaults()
{
    beginResetModel();
    extension_->restoreDefaultEngines();
    endResetModel();
}


Qt::DropActions EnginesModel::supportedDropActions() const
{
    return Qt::MoveAction;
}


bool EnginesModel::dropMimeData(const QMimeData *data, Qt::DropAction /*action*/,
                                int dstRow, int /*column*/, const QModelIndex &/*parent*/)
{
    QByteArray encoded = data->data("application/x-qabstractitemmodeldatalist");
    QDataStream stream(&encoded, QIODevice::ReadOnly);
    int srcRow = 0;
    if (!stream.atEnd())
        stream >> srcRow;
    moveRows(QModelIndex(), srcRow, 1, QModelIndex(), dstRow);
    return false;
}
