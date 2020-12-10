// Copyright (C) 2014-2020 Manuel Schneider

#include <QDir>
#include <QDirIterator>
#include <QFile>
#include <QFileSystemWatcher>
#include <QFutureWatcher>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QPointer>
#include <QSettings>
#include <QStandardPaths>
#include <QtConcurrent>
#include <QWidget>
#include <set>
#include <vector>
#include "ui_configwidget.h"
#include "extension.h"
#include "albert/util/standardactions.h"
#include "albert/util/standardindexitem.h"
#include "albert/util/offlineindex.h"
#include "xdg/iconlookup.h"
Q_LOGGING_CATEGORY(qlc, "chromium")
#define DEBG qCDebug(qlc,).noquote()
#define INFO qCInfo(qlc,).noquote()
#define WARN qCWarning(qlc,).noquote()
#define CRIT qCCritical(qlc,).noquote()
using namespace std;
using namespace Core;

namespace {
const char* EXT_ID = "org.albert.extension.chromebookmarks";
const char* CFG_FUZZY = "fuzzy";
const bool  DEF_FUZZY = false;
const char *CONFIG_LOCATIONS[] = {
    "BraveSoftware",
    "brave-browser",
    "chromium",
    "google-chrome"
};
}


/** ***************************************************************************/
/** ***************************************************************************/

class Chromium::Private
{
public:
    Private(Extension *q) : q(q) {}

    Extension *q;

    QPointer<QWidget> widget;
    QFileSystemWatcher fileSystemWatcher;
    QSet<QString> bookmarksFiles;

    vector<shared_ptr<Core::StandardIndexItem>> index;
    Core::OfflineIndex offlineIndex;
    QFutureWatcher<vector<shared_ptr<Core::StandardIndexItem>>> futureWatcher;


    void startIndexing() {

        // Never run concurrent
        if ( futureWatcher.future().isRunning() )
            return;

        // Run finishIndexing when the indexing thread finished
        futureWatcher.disconnect();
        QObject::connect(&futureWatcher, &QFutureWatcher<vector<shared_ptr<Core::StandardIndexItem>>>::finished,
                         std::bind(&Private::finishIndexing, this));

        // Run the indexer thread
        futureWatcher.setFuture(QtConcurrent::run(getBookmarks, bookmarksFiles));

        // Notification
        INFO << "Start indexing Chrome bookmarks.";
        emit q->statusInfo("Indexing bookmarks ...");

    }


    void finishIndexing() {

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
        for (auto filePath : bookmarksFiles)
            if (!fileSystemWatcher.files().contains(filePath))
                if(!fileSystemWatcher.addPath(filePath))
                    WARN << filePath << "can not be watched. Changes in this path will not be noticed.";

        // Notification
        auto msg = QString("%1 bookmarks indexed from %2.").arg(index.size())
                .arg(QStringList::fromSet(bookmarksFiles).join(", "));
        INFO << msg;
        emit q->statusInfo(msg);
    }


    static vector<shared_ptr<StandardIndexItem>> getBookmarks(const QSet<QString>& bookmarksFiles){

        vector<shared_ptr<StandardIndexItem>> bookmarkItems;

        // For each bookmarks file
        for (auto filePath : bookmarksFiles) {
            QFile f(filePath);
            if (f.open(QIODevice::ReadOnly)) {

                QString icon = XDG::IconLookup::iconPath({"www", "web-browser", "emblem-web"});
                icon = icon.isEmpty() ? ":favicon" : icon;

                // Read the bookmarks
                QJsonObject json = QJsonDocument::fromJson(f.readAll()).object();
                QJsonObject roots = json.value("roots").toObject();
                set<tuple<QString,QString,QString>> bookmarks;
                for (const QJsonValue &root : roots)
                    if (root.isObject())
                        getBookmarksRecursionHelper(root.toObject(), bookmarks);

                // Create items and add to results
                for (const auto& tuple : bookmarks){
                    QString guid = get<0>(tuple);
                    QString title = get<1>(tuple);
                    QString url = get<2>(tuple);
                    auto item = make_shared<StandardIndexItem>(
                            QString("%1.%2").arg(EXT_ID, guid), icon, title, url, title, Item::Urgency::Normal,
                            initializer_list<shared_ptr<Action>>{
                                make_shared<UrlAction>("Open URL", url),
                                make_shared<ClipAction>("Copy URL to clipboard", url)
                            },
                            initializer_list<Core::IndexableItem::IndexString>{
                                {title, UINT_MAX},
                                {url, UINT_MAX/2},
                            }
                    );
                    bookmarkItems.emplace_back(item);
                }
                f.close();
            } else
                WARN << "Could not open Chrome bookmarks file:" << filePath;
        }

        return bookmarkItems;
    }


    static void getBookmarksRecursionHelper(const QJsonObject &json, set<tuple<QString,QString,QString>> &output){
        QJsonValue type = json["type"];
        if (type != QJsonValue::Undefined){

            if (type.toString() == "folder")
                for (const QJsonValueRef child : json["children"].toArray())
                    getBookmarksRecursionHelper(child.toObject(), output);

            if (type.toString() == "url")
                output.emplace(json["guid"].toString(), json["name"].toString(), json["url"].toString());
        }
    }
};


/** ***************************************************************************/
/** ***************************************************************************/

Chromium::Extension::Extension()
    : Core::Extension(EXT_ID),
      Core::QueryHandler(Core::Plugin::id()),
      d(new Private(this)) {

    // Load settings
    d->offlineIndex.setFuzzy(settings().value(CFG_FUZZY, DEF_FUZZY).toBool());

    // Update index when a "Bookmark" file changed
    connect(&d->fileSystemWatcher, &QFileSystemWatcher::fileChanged,
            this, [this](){ d->startIndexing(); });

    // Find "Bookmarks" files, implicitly triggers an initial update
    updatePaths();

    registerQueryHandler(this);
}


/** ***************************************************************************/
Chromium::Extension::~Extension() {}


/** ***************************************************************************/
void Chromium::Extension::updatePaths() {

    QSet<QString> files;

    // Search for chromium based "Bookmarks" files
    QDir configDir = QDir(QStandardPaths::writableLocation(QStandardPaths::ConfigLocation));
    for (const QString &configLocation : CONFIG_LOCATIONS){
        QString root = configDir.filePath(configLocation);
        QDirIterator it(root, {"Bookmarks"}, QDir::Files, QDirIterator::Subdirectories);
        while (it.hasNext())
            files.insert(it.next());
    }

    // If paths changed update bookmarks
    if (d->bookmarksFiles != files){
        d->bookmarksFiles = files;
        d->startIndexing();
    }
}


/** ***************************************************************************/
QWidget *Chromium::Extension::widget(QWidget *parent) {
    if (d->widget.isNull()){
        d->widget = new QWidget(parent);

        Ui::ConfigWidget ui;
        ui.setupUi(d->widget);

        // Fuzzy
        ui.checkBox_fuzzy->setChecked(fuzzy());
        connect(ui.checkBox_fuzzy, &QCheckBox::toggled, this, &Extension::setFuzzy);

        // Status bar
        ( d->futureWatcher.isRunning() )
            ? ui.label_statusbar->setText("Indexing bookmarks ...")
            : ui.label_statusbar->setText(QString("%1 bookmarks indexed from %2.").arg(d->index.size()).
                                          arg(QStringList::fromSet(d->bookmarksFiles).join(", ")));
        connect(this, &Extension::statusInfo, ui.label_statusbar, &QLabel::setText);
    }
    return d->widget.data();
}


/** ***************************************************************************/
bool Chromium::Extension::fuzzy() {
    return d->offlineIndex.fuzzy();
}


/** ***************************************************************************/
void Chromium::Extension::setFuzzy(bool b) {
    settings().setValue(CFG_FUZZY, b);
    d->offlineIndex.setFuzzy(b);
}


/** ***************************************************************************/
void Chromium::Extension::handleQuery(Core::Query * query) const {

    const vector<shared_ptr<Core::IndexableItem>> &indexables = d->offlineIndex.search(query->string());

    vector<pair<shared_ptr<Core::Item>,uint>> results;
    for (const shared_ptr<Core::IndexableItem> &item : indexables)
        results.emplace_back(std::static_pointer_cast<Core::StandardIndexItem>(item), 0);

    query->addMatches(std::make_move_iterator(results.begin()),
                      std::make_move_iterator(results.end()));
}

