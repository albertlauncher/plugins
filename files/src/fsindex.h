// Copyright (c) 2022-2023 Manuel Schneider

#pragma once
#include "fsindexpath.h"
#include <QFutureWatcher>
#include <QString>
#include <map>
#include <memory>
#include <set>

class FsIndex : public QObject
{
    Q_OBJECT
public:
    FsIndex();
    ~FsIndex();

    const std::map<QString, std::unique_ptr<FsIndexPath>> &indexPaths() const;

    void addPath(std::unique_ptr<FsIndexPath> fsp);
    void removePath(const QString &path);

    void update(FsIndexPath *p = nullptr);

private:
    void updateThreaded(FsIndexPath *p);
    void runIndexer();
    QFutureWatcher<void> future_watcher;
    FsIndexPath *updating;
    std::set<FsIndexPath*> queue;
    bool abort;
    std::map<QString, std::unique_ptr<FsIndexPath>> index_paths_;  // DO NOT JUST REMOVE

signals:
    void status(const QString&);
    void updatedFinished();
};






















