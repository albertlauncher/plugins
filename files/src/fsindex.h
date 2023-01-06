// Copyright (c) 2022 Manuel Schneider

#pragma once
#include <QFileSystemWatcher>
#include <QFutureWatcher>
#include <QStringList>
#include <QTimer>
#include <set>

namespace albert{ class Item; }
class RootNode;
class QJsonObject;
class AbstractFileItem;


class FsIndexPath : public QObject
{
public:
    FsIndexPath(const QString &path);
    FsIndexPath(const QJsonObject &json);
    ~FsIndexPath();
    QJsonObject toJson() const;

    QString path() const;
    void update(const bool &abort, std::function<void(const QString&)> status);
    void items(std::vector<std::shared_ptr<AbstractFileItem>>&) const;

    const QStringList &nameFilters() const;
    const QStringList &mimeFilters() const;
    bool indexHidden() const;
    bool followSymlinks() const;
    uint8_t maxDepth() const;
    bool watchFileSystem() const;
    uint scanInterval() const;

    void setNameFilters(const QStringList&);
    void setMimeFilters(const QStringList&);
    void setIndexHidden(bool);
    void setFollowSymlinks(bool);
    void setMaxDepth(uint8_t);
    void setWatchFilesystem(bool);
    void setScanInterval(uint minutes);

private:
    void init();
    QStringList name_filters;
    QStringList mime_filters;
    uint8_t max_depth = 255;
    bool index_hidden_files = false;
    bool follow_symlinks = false;
    bool watch_fs = false;
    bool force_update = false;
    QTimer scan_interval_timer_;
    QFileSystemWatcher fs_watcher_;
    std::shared_ptr<RootNode> root_;
    std::shared_ptr<AbstractFileItem> self;

Q_OBJECT signals:
    void updateRequired(FsIndexPath*);
};


class FsIndex : public QObject
{
public:
    FsIndex();
    ~FsIndex();

    const std::map<QString,std::unique_ptr<FsIndexPath>> &indexPaths() const;

    bool addPath(FsIndexPath *path);
    void remPath(const QString &path);

    void update(FsIndexPath *p = nullptr);

private:
    void updateThreaded(FsIndexPath *p);
    void runIndexer();
    QFutureWatcher<void> future_watcher;
    FsIndexPath *updating;
    std::set<FsIndexPath*> queue;
    bool abort;
    std::map<QString,std::unique_ptr<FsIndexPath>> index_paths_;  // DO NOT JUST REMOVE

Q_OBJECT signals:
    void status(const QString&);
    void updatedFinished();

};






















