// Copyright (c) 2022-2024 Manuel Schneider

#include "configwidget.h"
#include "fileitems.h"
#include "plugin.h"
#include <QDir>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSettings>
#include <albert/extensionregistry.h>
#include <albert/logging.h>
#include <albert/standarditem.h>
#include <albert/util.h>
ALBERT_LOGGING_CATEGORY("files")
using namespace albert;
using namespace std;

const char* CFG_PATHS = "paths";
const char* CFG_MIME_FILTERS = "mimeFilters";
const QStringList DEF_MIME_FILTERS = { "inode/directory" };
const char* CFG_NAME_FILTERS = "nameFilters";
#if defined Q_OS_MACOS
const QStringList DEF_NAME_FILTERS = { "\\.DS_Store" };
#else
const QStringList DEF_NAME_FILTERS = {};
#endif
const char* CFG_INDEX_HIDDEN = "indexhidden";
const bool DEF_INDEX_HIDDEN = false;
const char* CFG_FOLLOW_SYMLINKS = "followSymlinks";
const bool DEF_FOLLOW_SYMLINKS = false;
const char* CFG_FS_WATCHES = "useFileSystemWatches";
const bool DEF_FS_WATCHES = false;
const char* CFG_MAX_DEPTH = "maxDepth";
const uint8_t DEF_MAX_DEPTH = 255;
const char* CFG_SCAN_INTERVAL = "scanInterval";
const uint DEF_SCAN_INTERVAL = 5;
const char* INDEX_FILE_NAME = "file_index.json";

Plugin::Plugin():
    homebrowser(fs_browsers_case_sensitive_),
    rootbrowser(fs_browsers_case_sensitive_)
{
    connect(&fs_index_, &FsIndex::status, this, &Plugin::statusInfo);
    connect(&fs_index_, &FsIndex::updatedFinished, this, [this](){ updateIndexItems(); });

    QJsonObject object;
    if (QFile file(createOrThrow(cacheLocation()).filePath(INDEX_FILE_NAME)); file.open(QIODevice::ReadOnly))
        object = QJsonDocument(QJsonDocument::fromJson(file.readAll())).object();

    auto s = settings();
    auto paths = s->value(CFG_PATHS, QStringList()).toStringList();
    for (const auto &path : paths){
        auto fsp = make_unique<FsIndexPath>(path);

        if (auto it = object.find(path); it != object.end())
            fsp->deserialize(it.value().toObject());

        s->beginGroup(path);
        fsp->setFollowSymlinks(s->value(CFG_FOLLOW_SYMLINKS, DEF_FOLLOW_SYMLINKS).toBool());
        fsp->setIndexHidden(s->value(CFG_INDEX_HIDDEN, DEF_INDEX_HIDDEN).toBool());
        fsp->setNameFilters(s->value(CFG_NAME_FILTERS, DEF_NAME_FILTERS).toStringList());
        fsp->setMimeFilters(s->value(CFG_MIME_FILTERS, DEF_MIME_FILTERS).toStringList());
        fsp->setMaxDepth(s->value(CFG_MAX_DEPTH, DEF_MAX_DEPTH).toUInt());
        fsp->setScanInterval(s->value(CFG_SCAN_INTERVAL, DEF_SCAN_INTERVAL).toUInt());
        fsp->setWatchFilesystem(s->value(CFG_FS_WATCHES, DEF_FS_WATCHES).toBool());
        s->endGroup();

        fs_index_.addPath(::move(fsp));
    }

    update_item = StandardItem::make(
        "scan_files",
        tr("Update index"),
        tr("Update the file index"),
        {":app_icon"},
        {{"scan_files", tr("Scan"), [this](){ fs_index_.update(); }}}
    );

    registry().registerExtension(&homebrowser);
    registry().registerExtension(&rootbrowser);
}

Plugin::~Plugin()
{
    registry().deregisterExtension(&homebrowser);
    registry().deregisterExtension(&rootbrowser);

    fs_index_.disconnect();

    auto s = settings();
    QStringList paths;
    QJsonObject object;
    for (auto &[path, fsp] : fs_index_.indexPaths()){
        paths << path;
        s->beginGroup(path);
        s->setValue(CFG_NAME_FILTERS, fsp->nameFilters());
        s->setValue(CFG_MIME_FILTERS, fsp->mimeFilters());
        s->setValue(CFG_INDEX_HIDDEN, fsp->indexHidden());
        s->setValue(CFG_FOLLOW_SYMLINKS, fsp->followSymlinks());
        s->setValue(CFG_MAX_DEPTH, fsp->maxDepth());
        s->setValue(CFG_FS_WATCHES, fsp->watchFileSystem());
        s->setValue(CFG_SCAN_INTERVAL, fsp->scanInterval());
        s->endGroup();
        object.insert(path, fsp->serialize());
    }
    s->setValue(CFG_PATHS, paths);

    if (QFile file(QDir(cacheLocation()).filePath(INDEX_FILE_NAME)); file.open(QIODevice::WriteOnly)) {
        DEBG << "Storing file index to" << file.fileName();
        file.write(QJsonDocument(object).toJson(QJsonDocument::Compact));
        file.close();
    } else
        WARN << "Couldn't write to file:" << file.fileName();
}

void Plugin::updateIndexItems()
{
    // Get file items
    vector<shared_ptr<FileItem>> items;
    for (auto &[path, fsp] : fs_index_.indexPaths())
        fsp->items(items);

    // Create index items
    vector<IndexItem> ii;
    for (auto &file_item : items)
        ii.emplace_back(file_item, file_item->name());

    // Add update item
    ii.emplace_back(update_item, update_item->text());

    // Add trash item
    auto item = StandardItem::make(
        "trash",
        tr("Trash"),
        tr("Your trash folder"),
        {"xdg:user-trash-full", "qsp:SP_TrashIcon"},
        {
#if defined(Q_OS_MAC)
            {
                "open", tr("Open trash"),
                [=](){ openUrl(QString("file://%1/.Trash").arg(QDir::homePath())); }
            },
            {
                "empty", tr("Empty trash"),
                [=](){ runDetachedProcess({"osascript", "-e", "tell application \"Finder\" to empty trash"}); }
            }
#elif defined(Q_OS_UNIX)
            {
                "open", tr("Open trash"),
                [=](){ openUrl(QStringLiteral("trash:///")); }
            }
#endif
        }
    );
    ii.emplace_back(item, item->text());

    setIndexItems(::move(ii));
}

QWidget *Plugin::buildConfigWidget() { return new ConfigWidget(this); }

const FsIndex &Plugin::fsIndex() { return fs_index_; }

void Plugin::addPath(const QString &path)
{
    auto fsp = make_unique<FsIndexPath>(path);
    fsp->setFollowSymlinks(DEF_FOLLOW_SYMLINKS);
    fsp->setIndexHidden(DEF_INDEX_HIDDEN);
    fsp->setNameFilters(DEF_NAME_FILTERS);
    fsp->setMimeFilters(DEF_MIME_FILTERS);
    fsp->setMaxDepth(DEF_MAX_DEPTH);
    fsp->setScanInterval(DEF_SCAN_INTERVAL);
    fsp->setWatchFilesystem(DEF_FS_WATCHES);
    fs_index_.addPath(::move(fsp));
}

void Plugin::removePath(const QString &path)
{
    fs_index_.removePath(path);
    updateIndexItems();
}
