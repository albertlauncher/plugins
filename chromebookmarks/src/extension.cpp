// Copyright (C) 2014-2018 Manuel Schneider

#include <QApplication>
#include <QClipboard>
#include <QDebug>
#include <QDesktopServices>
#include <QDir>
#include <QDirIterator>
#include <QFile>
#include <QFileInfo>
#include <QFileSystemWatcher>
#include <QFutureWatcher>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QPointer>
#include <QProcess>
#include <QSettings>
#include <QStandardPaths>
#include <QtConcurrent>
#include <QTimer>
#include <QUrl>
#include <functional>
#include <memory>
#include <vector>
#include "configwidget.h"
#include "extension.h"
#include "util/standardactions.h"
#include "util/standardindexitem.h"
#include "util/offlineindex.h"
#include "xdg/iconlookup.h"
using namespace std;
using namespace Core;

namespace {

const char* CFG_PATH  = "bookmarkfile";
const char* CFG_FUZZY = "fuzzy";
const bool  DEF_FUZZY = false;
const char *potentialExecutableNames[] = {"chromium",
                                          "chromium-browser",
                                          "chrome",
                                          "chrome-browser",
                                          "google-chrome",
                                          "google-chrome-beta",
                                          "google-chrome-stable",
                                          "google-chrome-unstable"};
const char *potentialConfigLocations[] = {"chromium",
                                          "google-chrome"};

/** ***************************************************************************/
vector<shared_ptr<StandardIndexItem>> indexChromeBookmarks(QString executable, const QString &bookmarksPath) {

    // Build a new index
    vector<shared_ptr<StandardIndexItem>> bookmarks;

    QString icon = XDG::IconLookup::iconPath({"www", "web-browser", "emblem-web"});
    icon = icon.isEmpty() ? ":favicon" : icon;

    // Define a recursive bookmark indexing lambda
    std::function<void(const QJsonObject &json)> rec_bmsearch =
            [&rec_bmsearch, &bookmarks, &icon, &executable](const QJsonObject &json) {
        QJsonValue type = json["type"];
        if (type == QJsonValue::Undefined)
            return;
        if (type.toString() == "folder"){
            QJsonArray jarr = json["children"].toArray();
            for (const QJsonValue &i : jarr)
                rec_bmsearch(i.toObject());
        }
        if (type.toString() == "url") {
            QString name = json["name"].toString();
            QString urlstr = json["url"].toString();

            vector<IndexableItem::IndexString> indexStrings;
            QUrl url(urlstr);
            QString host = url.host();
            indexStrings.emplace_back(name, UINT_MAX);
            indexStrings.emplace_back(host.left(host.size()-url.topLevelDomain().size()), UINT_MAX/2);

            shared_ptr<StandardIndexItem> item = std::make_shared<StandardIndexItem>(json["id"].toString());
            item->setText(name);
            item->setCompletion(name);
            item->setSubtext(urlstr);
            item->setIconPath(icon);
            item->setIndexKeywords(std::move(indexStrings));
            item->addAction(make_shared<ProcAction>("Open URL",
                                                    QStringList() << executable << urlstr));
            item->addAction(make_shared<ProcAction>("Open URL in new window",
                                                    QStringList() << executable << "--new-window"  << urlstr));
            item->addAction(make_shared<ClipAction>("Copy URL to clipboard", urlstr));

            bookmarks.push_back(std::move(item));
        }
    };

    QFile f(bookmarksPath);
    if (!f.open(QIODevice::ReadOnly)) {
        qWarning() << qPrintable(QString("Could not open Chrome bookmarks file '%1'.").arg(bookmarksPath));
        return vector<shared_ptr<StandardIndexItem>>();
    }

    QJsonObject json = QJsonDocument::fromJson(f.readAll()).object();
    QJsonObject roots = json.value("roots").toObject();
    for (const QJsonValue &i : roots)
        if (i.isObject())
            rec_bmsearch(i.toObject());

    f.close();

    return bookmarks;
}

}



/** ***************************************************************************/
/** ***************************************************************************/
/** ***************************************************************************/
/** ***************************************************************************/
class ChromeBookmarks::Private
{
public:
    Private(Extension *q) : q(q) {}

    Extension *q;

    QPointer<ConfigWidget> widget;
    QFileSystemWatcher fileSystemWatcher;
    QString bookmarksFile;
    QString executable;

    vector<shared_ptr<Core::StandardIndexItem>> index;
    Core::OfflineIndex offlineIndex;
    QFutureWatcher<vector<shared_ptr<Core::StandardIndexItem>>> futureWatcher;

    void finishIndexing();
    void startIndexing();
};


/** ***************************************************************************/
void ChromeBookmarks::Private::startIndexing() {

    // Never run concurrent
    if ( futureWatcher.future().isRunning() )
        return;

    // Run finishIndexing when the indexing thread finished
    futureWatcher.disconnect();
    QObject::connect(&futureWatcher, &QFutureWatcher<vector<shared_ptr<Core::StandardIndexItem>>>::finished,
                     std::bind(&Private::finishIndexing, this));

    // Run the indexer thread
    futureWatcher.setFuture(QtConcurrent::run(indexChromeBookmarks, executable, bookmarksFile));

    // Notification
    qInfo() << "Start indexing Chrome bookmarks.";
    emit q->statusInfo("Indexing bookmarks ...");

}


/** ***************************************************************************/
void ChromeBookmarks::Private::finishIndexing() {

    // Get the thread results
    index = futureWatcher.future().result();

    // Rebuild the offline index
    offlineIndex.clear();
    for (const auto &item : index)
        offlineIndex.add(item);

    /*
     * Finally update the watches (maybe folders changed)
     * Note that QFileSystemWatcher stops monitoring files once they have been
     * renamed or removed from disk, and directories once they have been removed
     * from disk.
     * Chromium seems to mv the file (inode change).
     */
    if ( fileSystemWatcher.files().empty() )
        if( !fileSystemWatcher.addPath(bookmarksFile))
            qWarning() << qPrintable(QString("%1 can not be watched. Changes in this path will not be noticed.").arg(bookmarksFile));

    // Notification
    qInfo() << qPrintable(QString("Indexed %1 Chrome bookmarks.").arg(index.size()));
    emit q->statusInfo(QString("%1 bookmarks indexed.").arg(index.size()));
}


/** ***************************************************************************/
/** ***************************************************************************/
/** ***************************************************************************/
/** ***************************************************************************/
ChromeBookmarks::Extension::Extension()
    : Core::Extension("org.albert.extension.chromebookmarks"),
      Core::QueryHandler(Core::Plugin::id()),
      d(new Private(this)) {


    // Find executable
    d->executable = QStandardPaths::findExecutable("chromium");
    for (auto &name : potentialExecutableNames) {
        d->executable = QStandardPaths::findExecutable(name);;
        if (!d->executable.isEmpty())
            break;
    }
    if (d->executable.isEmpty())
        throw "Chrome/ium executable not found.";

    // Load settings
    d->offlineIndex.setFuzzy(settings().value(CFG_FUZZY, DEF_FUZZY).toBool());

    // Load and set a valid path
    QVariant v = settings().value(CFG_PATH);
    if (v.isValid() && v.canConvert(QMetaType::QString) && QFileInfo(v.toString()).exists())
        setPath(v.toString());
    else
        restorePath();

    // If the path changed write it to the settings
    connect(this, &Extension::pathChanged, [this](const QString& path){
        settings().setValue(CFG_PATH, path);
    });

    // Update index if bookmark file changed
    connect(&d->fileSystemWatcher, &QFileSystemWatcher::fileChanged,
            this, &Extension::updateIndex);

    // Update index if bookmark file's path changed
    connect(this, &Extension::pathChanged,
            this, &Extension::updateIndex);

    // Trigger an initial update
    updateIndex();

    registerQueryHandler(this);
}


/** ***************************************************************************/
ChromeBookmarks::Extension::~Extension() {}


/** ***************************************************************************/
QWidget *ChromeBookmarks::Extension::widget(QWidget *parent) {
    if (d->widget.isNull()){
        d->widget = new ConfigWidget(parent);

        // Paths
        d->widget->ui.lineEdit_path->setText(d->bookmarksFile);
        connect(d->widget.data(), &ConfigWidget::requestEditPath, this, &Extension::setPath);
        connect(this, &Extension::pathChanged, d->widget->ui.lineEdit_path, &QLineEdit::setText);

        // Fuzzy
        d->widget->ui.checkBox_fuzzy->setChecked(fuzzy());
        connect(d->widget->ui.checkBox_fuzzy, &QCheckBox::toggled, this, &Extension::setFuzzy);

        // Status bar
        ( d->futureWatcher.isRunning() )
            ? d->widget->ui.label_statusbar->setText("Indexing bookmarks ...")
            : d->widget->ui.label_statusbar->setText(QString("%1 bookmarks indexed.").arg(d->index.size()));
        connect(this, &Extension::statusInfo, d->widget->ui.label_statusbar, &QLabel::setText);
    }
    return d->widget;
}


/** ***************************************************************************/
void ChromeBookmarks::Extension::handleQuery(Core::Query * query) const {

    const vector<shared_ptr<Core::IndexableItem>> &indexables = d->offlineIndex.search(query->string());

    vector<pair<shared_ptr<Core::Item>,uint>> results;
    for (const shared_ptr<Core::IndexableItem> &item : indexables)
        results.emplace_back(std::static_pointer_cast<Core::StandardIndexItem>(item), 0);

    query->addMatches(std::make_move_iterator(results.begin()),
                      std::make_move_iterator(results.end()));
}


/** ***************************************************************************/
const QString &ChromeBookmarks::Extension::path() {
    return d->bookmarksFile;
}


/** ***************************************************************************/
void ChromeBookmarks::Extension::setPath(const QString &path) {

    QFileInfo fi(path);
    if (!(fi.exists() && fi.isFile()))
        return;

    d->bookmarksFile = path;

    emit pathChanged(path);
}


/** ***************************************************************************/
void ChromeBookmarks::Extension::restorePath() {
    // Find a bookmark file (Take first one)
    for (const QString &browser : potentialConfigLocations){
        QString root = QDir(QStandardPaths::writableLocation(QStandardPaths::ConfigLocation)).filePath(browser);
        QDirIterator it(root, {"Bookmarks"}, QDir::Files, QDirIterator::Subdirectories);
        while (it.hasNext()) {
            setPath(it.next());
            return;
        }
    }
}


/** ***************************************************************************/
bool ChromeBookmarks::Extension::fuzzy() {
    return d->offlineIndex.fuzzy();
}


/** ***************************************************************************/
void ChromeBookmarks::Extension::updateIndex() {
    d->startIndexing();
}


/** ***************************************************************************/
void ChromeBookmarks::Extension::setFuzzy(bool b) {
    settings().setValue(CFG_FUZZY, b);
    d->offlineIndex.setFuzzy(b);
}

