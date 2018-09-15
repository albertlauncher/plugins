// Copyright (C) 2014-2017 Manuel Schneider

#include <QDebug>
#include <QDir>
#include <QDirIterator>
#include <QFutureWatcher>
#include <QMessageBox>
#include <QObject>
#include <QPointer>
#include <QSettings>
#include <QStandardPaths>
#include <QtConcurrent>
#include <QThreadPool>
#include <QTimer>
#include <memory>
#include <functional>
#include <vector>
#include <set>
#include "configwidget.h"
#include "file.h"
#include "standardfile.h"
#include "extension.h"
#include "indextreenode.h"
#include "util/offlineindex.h"
#include "util/standarditem.h"
#include "util/standardactions.h"
#include "xdg/iconlookup.h"
using namespace Core;
using namespace std;


namespace {

const char* CFG_PATHS           = "paths";
const char* CFG_FILTERS         = "filters";
const QStringList DEF_FILTERS   = { "inode/directory", "application/*" };
const char* CFG_FUZZY           = "fuzzy";
const bool  DEF_FUZZY           = false;
const char* CFG_INDEX_HIDDEN    = "indexhidden";
const bool  DEF_INDEX_HIDDEN    = false;
const char* CFG_FOLLOW_SYMLINKS = "follow_symlinks";
const bool  DEF_FOLLOW_SYMLINKS = false;
const char* CFG_SCAN_INTERVAL   = "scan_interval";
const uint  DEF_SCAN_INTERVAL   = 15;



class OfflineIndexBuilderVisitor : public Files::Visitor {
    Core::OfflineIndex &offlineIndex;
public:
    OfflineIndexBuilderVisitor(Core::OfflineIndex &offlineIndex)
        : offlineIndex(offlineIndex) { }

    void visit(Files::IndexTreeNode *node) override {
        for ( const shared_ptr<Files::File> &item : node->items() )
            offlineIndex.add(item);
    }
};


class CounterVisitor : public Files::Visitor {
public:
    uint itemCount = 0;
    uint dirCount = 0;
    void visit(Files::IndexTreeNode *node) override {
        ++dirCount;
        itemCount += node->items().size();
    }
};

}



/** ***************************************************************************/
/** ***************************************************************************/
/** ***************************************************************************/
/** ***************************************************************************/
class Files::Private
{
public:
    Private(Extension *q) : q(q), abort(false), rerun(false) {}

    Extension *q;

    QPointer<ConfigWidget> widget;

    QStringList indexRootDirs;
    IndexSettings indexSettings;
    vector<shared_ptr<IndexTreeNode>> indexTrees;
    unique_ptr<QFutureWatcher<Core::OfflineIndex*>> futureWatcher;
    Core::OfflineIndex offlineIndex;
    QTimer indexIntervalTimer;
    bool abort;
    bool rerun;


    void finishIndexing();
    void startIndexing();
    Core::OfflineIndex *indexFiles();
};



/** ***************************************************************************/
void Files::Private::startIndexing() {

    // Abort and rerun
    if ( futureWatcher ) {
        emit q->statusInfo("Waiting for indexer to shut down ...");
        abort = true;
        rerun = true;
        return;
    }

    // Run finishIndexing when the indexing thread finished
    futureWatcher.reset(new QFutureWatcher<Core::OfflineIndex*>);
    QObject::connect(futureWatcher.get(), &QFutureWatcher<Core::OfflineIndex*>::finished,
                     [this](){ this->finishIndexing(); });

    // Restart the timer (Index update may have been started manually)
    if (indexIntervalTimer.interval() != 0)
        indexIntervalTimer.start();

    // Run the indexer thread
    qInfo() << "Start indexing files.";
    futureWatcher->setFuture(QtConcurrent::run(this, &Private::indexFiles));

    // Notification
    emit q->statusInfo("Indexing files ...");
}



/** ***************************************************************************/
void Files::Private::finishIndexing() {

    // In case of abortion the returned data is invalid
    if ( !abort ) {
        OfflineIndex *retval = futureWatcher->future().result();
        if (retval) {
            offlineIndex = std::move(*retval);
            delete retval;
        }

        // Notification
        CounterVisitor counterVisitor;
        for (const auto & tree : indexTrees )
            tree->accept(counterVisitor);
        qInfo() << qPrintable(QString("Indexed %1 files in %2 directories.")
                              .arg(counterVisitor.itemCount).arg(counterVisitor.dirCount));
        emit q->statusInfo(QString("Indexed %1 files in %2 directories.")
                           .arg(counterVisitor.itemCount).arg(counterVisitor.dirCount));
    }

    futureWatcher.reset();
    abort = false;

    if ( rerun ) {
        rerun = false;
        startIndexing();
    }
}



/** ***************************************************************************/
OfflineIndex* Files::Private::indexFiles() {

    // Remove the subtrees not wanted anymore
    auto it = indexTrees.begin();
    while ( it != indexTrees.end() ) {
        if ( indexRootDirs.contains((*it)->path()) )
            ++it;
        else {
            (*it)->removeDownlinks();
            it = indexTrees.erase(it);
        }
    }

    // Start the indexing
    for ( const QString &rootDir : indexRootDirs ) {

        emit q->statusInfo(QString("Indexing %1…").arg(rootDir));

        // If this root dir does not exist create it
        auto it = find_if(indexTrees.begin(), indexTrees.end(),
                          [&rootDir](const shared_ptr<IndexTreeNode>& tree){ return tree->path() == rootDir; });
        if ( it == indexTrees.end() ) {
            indexTrees.push_back(make_shared<IndexTreeNode>(rootDir));
            indexTrees.back()->update(abort, indexSettings);
        }
        else
            (*it)->update(abort, indexSettings);


        if ( abort )
            return nullptr;
    }

    // Serialize data
    qDebug() << "Serializing files…";
    emit q->statusInfo("Serializing index data…");
    QFile file(q->cacheLocation().filePath("fileindex.json"));
    if (file.open(QIODevice::WriteOnly)) {
        QJsonArray array;
        for (auto& tree : this->indexTrees)
            array.push_back(tree->serialize());
        QJsonDocument doc(array);
        file.write(doc.toJson(QJsonDocument::Compact));
        file.close();
    }
    else
        qWarning() << "Couldn't write to file:" << file.fileName();


    // Build offline index
    qDebug() << "Building inverted file index…";
    emit q->statusInfo("Building inverted index…");
    Core::OfflineIndex *offline = new Core::OfflineIndex(indexSettings.fuzzy());
    OfflineIndexBuilderVisitor visitor(*offline);
    for (auto& tree : this->indexTrees)
        tree->accept(visitor);
    return offline;
}


/** ***************************************************************************/
/** ***************************************************************************/
/** ***************************************************************************/
/** ***************************************************************************/
Files::Extension::Extension()
    : Core::Extension("org.albert.extension.files"),
      Core::QueryHandler(Core::Plugin::id()),
      d(new Private(this)) {

    registerQueryHandler(this);

    // Load settings
    d->indexSettings.setFilters(settings().value(CFG_FILTERS, DEF_FILTERS).toStringList());
    d->indexSettings.setIndexHidden(settings().value(CFG_INDEX_HIDDEN, DEF_INDEX_HIDDEN).toBool());
    d->indexSettings.setFollowSymlinks(settings().value(CFG_FOLLOW_SYMLINKS, DEF_FOLLOW_SYMLINKS).toBool());
    d->indexSettings.setFuzzy(settings().value(CFG_FUZZY, DEF_FUZZY).toBool());
    d->indexSettings.setForceUpdate(false);
    d->offlineIndex.setFuzzy(d->indexSettings.fuzzy());
    d->indexIntervalTimer.setInterval(settings().value(CFG_SCAN_INTERVAL, DEF_SCAN_INTERVAL).toInt()*60000); // Will be started in the initial index update
    d->indexRootDirs = settings().value(CFG_PATHS, QDir::homePath()).toStringList();

    // Index timer
    connect(&d->indexIntervalTimer, &QTimer::timeout, this, &Extension::updateIndex);

    // If the root dirs change write it to the settings
    connect(this, &Extension::pathsChanged, [this](const QStringList& dirs){
        settings().setValue(CFG_PATHS, dirs);
    });

    // Deserialize data
    qDebug() << "Loading file index from cache.";
    QFile file(cacheLocation().filePath("fileindex.json"));
    if ( file.exists() ) {
        if (file.open(QIODevice::ReadOnly)) {
            QJsonDocument loadDoc{QJsonDocument::fromJson(file.readAll())};
            for ( const QJsonValueRef value : loadDoc.array()){
                d->indexTrees.push_back(make_shared<IndexTreeNode>());  // Invalid node
                d->indexTrees.back()->deserialize(value.toObject());
            }
            file.close();

            // Build offline index
            qDebug() << "Building inverted file index.";
            OfflineIndexBuilderVisitor visitor(d->offlineIndex);
            for (auto& tree : d->indexTrees)
                tree->accept(visitor);
        }
        else
            qWarning() << "Could not read from file: " << file.fileName();
    }

    // Trigger an initial update
    updateIndex();
}



/** ***************************************************************************/
Files::Extension::~Extension() {

    // The indexer thread has sideeffects wait for termination
    d->abort = true;
    d->rerun = false;
    if ( d->futureWatcher ){
        disconnect(d->futureWatcher.get(), 0, 0, 0);
        d->futureWatcher->waitForFinished();
    }
}



/** ***************************************************************************/
QWidget *Files::Extension::widget(QWidget *parent) {
    if (d->widget.isNull())
        d->widget = new ConfigWidget(this, parent);
    return d->widget;
}



/** ***************************************************************************/
void Files::Extension::handleQuery(Core::Query * query) const {

    if ( query->trigger()=="/" || query->trigger()=="~" ) {

        QFileInfo queryFileInfo(query->rawString());

        // Substitute tilde
        if ( query->rawString()[0] == '~' )
            queryFileInfo.setFile(QDir::homePath()+query->string());

        // Get all matching files
        QFileInfo pathInfo(queryFileInfo.path());
        if ( pathInfo.exists() && pathInfo.isDir() ) {

            QMimeDatabase mimeDatabase;
            QDir dir(pathInfo.filePath());
            QString commonPrefix;
            QString queryFileName = queryFileInfo.fileName();
            vector<shared_ptr<StandardItem>> items;

            for (const QFileInfo& fileInfo : dir.entryInfoList(QDir::AllEntries|QDir::Hidden|QDir::NoDotAndDotDot,
                                                               QDir::DirsFirst|QDir::Name|QDir::IgnoreCase) ) {
                QString fileName = fileInfo.fileName();

                if ( fileName.startsWith(queryFileName) ) {

                    if (fileInfo.isDir())
                        fileName.append(QDir::separator());

                    if (commonPrefix.isNull())
                        commonPrefix = fileName;
                    else {
                        auto pair = mismatch(commonPrefix.begin() , commonPrefix.end(), fileName.begin(), fileName.end());
                        commonPrefix.resize(distance(commonPrefix.begin(), pair.first));
                    }

                    QMimeType mimetype = mimeDatabase.mimeTypeForFile(fileInfo.filePath());
                    QString icon = XDG::IconLookup::iconPath({mimetype.iconName(), mimetype.genericIconName(), "unknown"});
                    if (icon.isEmpty())
                        icon = (mimetype.iconName() == "inode-directory") ? ":directory" : ":unknown";

                    auto item = make_shared<StandardItem>(fileInfo.filePath(),
                                                          icon,
                                                          fileName,
                                                          fileInfo.filePath());
                    item->setActions(File::buildFileActions(fileInfo.filePath()));
                    items.push_back(move(item));
                }
            }
            for (auto &item : items) {
                item->setCompletion(dir.filePath(commonPrefix));
                query->addMatch(std::move(item));
            }
        }
    }
    else
    {
        if ( query->string().isEmpty() )
            return;

        if ( QString("albert scan files").startsWith(query->string()) ) {

            auto item = make_shared<StandardItem>("files.action.index");
            item->setText("albert scan files");
            item->setSubtext("Update the file index");
            item->setIconPath(":app_icon");
            // Const cast is fine since the action will not be called here
            item->addAction(make_shared<FuncAction>("Update the file index",
                                                    [this](){ const_cast<Extension*>(this)->updateIndex();}));

            query->addMatch(move(item));
        }

        // Search for matches
        const vector<shared_ptr<IndexableItem>> &indexables = d->offlineIndex.search(query->string());

        // Add results to query
        vector<pair<shared_ptr<Core::Item>,uint>> results;
        for (const shared_ptr<Core::IndexableItem> &item : indexables)
            // TODO `Search` has to determine the relevance. Set to 0 for now
            results.emplace_back(static_pointer_cast<File>(item), 0);

        query->addMatches(make_move_iterator(results.begin()),
                          make_move_iterator(results.end()));
    }
}



/** ***************************************************************************/
const QStringList &Files::Extension::paths() const {
    return d->indexRootDirs;
}



/** ***************************************************************************/
void Files::Extension::setPaths(const QStringList &paths) {

    if (d->indexRootDirs == paths)
        return;

    d->indexRootDirs.clear();

    // Check sanity and add path
    for ( const QString& path : paths ) {

        QFileInfo fileInfo(path);
        QString absPath = fileInfo.absoluteFilePath();

        if (d->indexRootDirs.contains(absPath)) {
            qWarning() << QString("Duplicate paths: %1.").arg(path);
            continue;
        }

        if (!fileInfo.exists()) {
            qWarning() << QString("Path does not exist: %1.").arg(path);
            continue;
        }

        if(!fileInfo.isDir()) {
            qWarning() << QString("Path is not a directory: %1.").arg(path);
            continue;
        }

        d->indexRootDirs << absPath;
    }

    sort(d->indexRootDirs.begin(), d->indexRootDirs.end());

    emit pathsChanged(d->indexRootDirs);

    // Store to settings
    settings().setValue(CFG_PATHS, d->indexRootDirs);

}



/** ***************************************************************************/
void Files::Extension::restorePaths() {
    // Add standard path
    d->indexRootDirs.clear();
    d->indexRootDirs << QStandardPaths::writableLocation(QStandardPaths::HomeLocation);
    emit pathsChanged(d->indexRootDirs);
}



/** ***************************************************************************/
void Files::Extension::updateIndex() {
    d->startIndexing();
}



/** ***************************************************************************/
bool Files::Extension::indexHidden() const {
    return d->indexSettings.indexHidden();
}



/** ***************************************************************************/
void Files::Extension::setIndexHidden(bool b)  {
    settings().setValue(CFG_INDEX_HIDDEN, b);
    d->indexSettings.setIndexHidden(b);
}



/** ***************************************************************************/
bool Files::Extension::followSymlinks() const {
    return d->indexSettings.followSymlinks();
}



/** ***************************************************************************/
void Files::Extension::setFollowSymlinks(bool b)  {
    settings().setValue(CFG_FOLLOW_SYMLINKS, b);
    d->indexSettings.setFollowSymlinks(b);
}



/** ***************************************************************************/
unsigned int Files::Extension::scanInterval() const {
    return static_cast<uint>(d->indexIntervalTimer.interval()/60000);
}



/** ***************************************************************************/
void Files::Extension::setScanInterval(uint minutes) {
    settings().setValue(CFG_SCAN_INTERVAL, minutes);
    (minutes == 0) ? d->indexIntervalTimer.stop()
                   : d->indexIntervalTimer.start(static_cast<int>(minutes*60000));
}



/** ***************************************************************************/
bool Files::Extension::fuzzy() const {
    return d->offlineIndex.fuzzy();
}



/** ***************************************************************************/
void Files::Extension::setFuzzy(bool b) {
    settings().setValue(CFG_FUZZY, b);
    d->offlineIndex.setFuzzy(b);
}



/** ***************************************************************************/
QStringList Files::Extension::filters() const {
    QStringList retval;
    for ( auto const & regex : d->indexSettings.filters() )
        retval.push_back(regex.pattern());
    return retval;
}



/** ***************************************************************************/
void Files::Extension::setFilters(const QStringList &filters) {
    settings().setValue(CFG_FILTERS, filters);
    d->indexSettings.setFilters(filters);
}
