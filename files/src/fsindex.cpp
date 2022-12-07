// Copyright (c) 2022 Manuel Schneider

#include "albert/logging.h"
#include "fileitems.h"
#include "fsindex.h"
#include "fsindexnodes.h"
#include <QFileInfo>
#include <QtConcurrent>
using namespace std;


void FsIndexPath::init()
{
    connect(&fs_watcher_, &QFileSystemWatcher::directoryChanged,
            this, [this](){ emit updateRequired(this); });
    connect(&scan_interval_timer_, &QTimer::timeout,
            this, [this](){ emit updateRequired(this); });

    // Be tolerant but warn
    if (QFileInfo fi(root_->filePath()); !fi.exists())
        WARN << QString("Root path does not exist: %1.").arg(fi.absolutePath());
    else if (!fi.isDir())
        WARN << QString("Root path is not a directory: %1.").arg(fi.absolutePath());

    self = make_shared<StandardFile>(root_->filePath(), DirNode::dirMimeType());
}

FsIndexPath::FsIndexPath(const QString &path) : root_(RootNode::make(path)){ init(); }

FsIndexPath::FsIndexPath(const QJsonObject &json) : root_(RootNode::fromJson(json)){ init(); }

FsIndexPath::~FsIndexPath() = default;

QJsonObject FsIndexPath::toJson() const { return root_->toJson(); }

QString FsIndexPath::path() const { return root_->filePath(); }

void FsIndexPath::update(const bool &abort, std::function<void(const QString &)> status)
{
    IndexSettings s;
    for (const auto &pattern : name_filters)
        s.name_filters.emplace_back(pattern);
    for (const auto &pattern : mime_filters)
        s.mime_filters.emplace_back(QRegularExpression::fromWildcard(pattern,
                                                                     Qt::CaseSensitive,
                                                                     QRegularExpression::UnanchoredWildcardConversion));
    s.index_hidden_files = index_hidden_files;
    s.follow_symlinks = follow_symlinks;
    s.max_depth = max_depth;
    s.forced = force_update;
    std::set<QString> indexed_dirs;

    root_->update(root_, abort, status, s, indexed_dirs, 1);

    status(QString("Indexed %1 directories in %2.").arg(indexed_dirs.size()).arg(path()));

    if (s.forced && !abort) // In case of successful forced run clear force flag
        force_update = false;
}

void FsIndexPath::items(vector<shared_ptr<AbstractFileItem>> &items) const
{
    items.emplace_back(self);
    root_->items(items);
}

const QStringList &FsIndexPath::nameFilters() const { return name_filters; }

const QStringList &FsIndexPath::mimeFilters() const { return mime_filters; }

bool FsIndexPath::indexHidden() const { return index_hidden_files; }

bool FsIndexPath::followSymlinks() const { return follow_symlinks; }

uint8_t FsIndexPath::maxDepth() const { return max_depth; }

bool FsIndexPath::watchFileSystem() const { return watch_fs; }

uint FsIndexPath::scanInterval() const { return (uint)(scan_interval_timer_.interval()/60000); }

void FsIndexPath::setNameFilters(const QStringList &val)
{
    name_filters = val;
    force_update = true;
    emit updateRequired(this);
}

void FsIndexPath::setMimeFilters(const QStringList &val)
{
    mime_filters = val;
    force_update = true;
    emit updateRequired(this);
}

void FsIndexPath::setIndexHidden(bool val)
{
    index_hidden_files = val;
    force_update = true;
    emit updateRequired(this);
}

void FsIndexPath::setFollowSymlinks(bool val)
{
    follow_symlinks = val;
    force_update = true;
    emit updateRequired(this);
}

void FsIndexPath::setMaxDepth(uint8_t val)
{
    max_depth = val;
    force_update = true;
    emit updateRequired(this);
}

void FsIndexPath::setWatchFilesystem(bool val)
{
    watch_fs = val;
    if (val){
        std::vector<std::shared_ptr<DirNode>> nodes;
        root_->nodes(nodes);
        QStringList l;
        for (auto &node : nodes)
            l << node->filePath();
        l << root_->filePath();
        fs_watcher_.addPaths(l);
    } else {
        if (!fs_watcher_.directories().empty())
            fs_watcher_.removePaths(fs_watcher_.directories());
    }
}

void FsIndexPath::setScanInterval(uint minutes)
{
    if (minutes)
        scan_interval_timer_.start((int)(minutes*60000));
    else
        scan_interval_timer_.stop();
}


FsIndex::FsIndex(): abort(false)
{
    QObject::connect(&future_watcher, &QFutureWatcher<void>::finished, [this](){
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
{
    return index_paths_;
}

bool FsIndex::addPath(FsIndexPath* fsp)
{
    const auto &[it, success] = index_paths_.emplace(fsp->path(), fsp);
    connect(it->second.get(), &FsIndexPath::updateRequired, this, &FsIndex::updateThreaded);
    return success;
}

void FsIndex::remPath(const QString &path)
{
    auto &fsp = index_paths_.at(path);
    disconnect(fsp.get(), &FsIndexPath::updateRequired, this, &FsIndex::updateThreaded);
    queue.erase(fsp.get());
    if (fsp.get() == updating){
        abort = true;
        future_watcher.waitForFinished();
    }
    index_paths_.erase(path);
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
        auto *fspath = *queue.begin();
        queue.erase(queue.begin());
        INFO << "Indexing" << fspath->path();
        future_watcher.setFuture(QtConcurrent::run([this, fsp=fspath](){
            try{
                fsp->update(abort, [this](const QString &s){ emit status(s);});
            } catch(const exception &e){
                CRIT << "Indexer crashed" << e.what();
            }
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

