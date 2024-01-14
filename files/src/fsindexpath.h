// Copyright (c) 2022-2023 Manuel Schneider

#pragma once
#include <QFileSystemWatcher>
#include <QStringList>
#include <QTimer>
#include <functional>
#include <memory>
#include <vector>
class FileItem;
class QJsonObject;
class RootNode;


class FsIndexPath : public QObject
{
    Q_OBJECT
public:
    FsIndexPath(const QString &path);
    ~FsIndexPath();

    QJsonObject serialize() const;
    void deserialize(const QJsonObject &json);

    QString path() const;
    void update(const bool &abort, std::function<void(const QString&)> status);
    void items(std::vector<std::shared_ptr<FileItem>>&) const;

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
    std::shared_ptr<FileItem> self;

signals:
    void updateRequired(FsIndexPath*);
};








