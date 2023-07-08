// Copyright (c) 2022 Manuel Schneider

#include "albert/logging.h"
#include "fsindex.h"
#include <QtConcurrent>
using namespace std;

FsIndex::FsIndex(): abort(false)
{
    QObject::connect(&future_watcher, &QFutureWatcher<void>::finished, this, [this](){
        updating = nullptr;
        if (queue.empty())
            emit updatedFinished();
        else
            runIndexer();
    });
}

FsIndex::~FsIndex()
{
    future_watcher.disconnect();
    abort = true;
    if (future_watcher.isRunning()){
        WARN << "Busy wait for file indexer.";
        future_watcher.waitForFinished();
    }
}

const map<QString,unique_ptr<FsIndexPath>> &FsIndex::indexPaths() const
{ return index_paths_; }

void FsIndex::addPath(unique_ptr<FsIndexPath> fsp)
{
    const auto &[it, success] = index_paths_.emplace(fsp->path(), ::move(fsp));
    if (success){
        connect(it->second.get(), &FsIndexPath::updateRequired, this, &FsIndex::updateThreaded);
        updateThreaded(it->second.get());
    }
}

void FsIndex::removePath(const QString &path)
{
    try {
        auto &fsp = index_paths_.at(path);
        disconnect(fsp.get(), &FsIndexPath::updateRequired, this, &FsIndex::updateThreaded);
        queue.erase(fsp.get());
        if (fsp.get() == updating){
            abort = true;
            future_watcher.waitForFinished();
        }
        index_paths_.erase(path);
    } catch (const out_of_range&) {
        CRIT << "Logic error: Removed non existing path.";
    }
}

void FsIndex::updateThreaded(FsIndexPath *p)
{
    queue.insert(p);
    if (updating == p)
        abort = true;
    runIndexer();
}

void FsIndex::runIndexer()
{
    if (!future_watcher.isRunning() && !queue.empty()){
        updating = *queue.begin();
        queue.erase(queue.begin());
        INFO << "Indexing" << updating->path();
        future_watcher.setFuture(QtConcurrent::run([this, fsp=updating](){
            try{
                fsp->update(abort, [this](const QString &s){ emit status(s);});
            } catch(const exception &e){
                CRIT << "Indexer crashed" << e.what();
            }
            abort = false;
        }));
    }
}

void FsIndex::update(FsIndexPath *p)
{
    if (p)
        updateThreaded(p);
    else
        for (auto &[_, fsp] : index_paths_)
            updateThreaded(fsp.get());
}

