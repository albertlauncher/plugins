// Copyright (C) 2014-2020 Manuel Schneider

#include <QDir>
#include <QDirIterator>
#include <QFile>
#include <QFileDialog>
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
const char* EXT_ID = "org.albert.extension.chromium";
const char* CFG_FUZZY = "fuzzy";
const bool  DEF_FUZZY = false;
const char* CFG_BOOKMARKS_PATH = "bookmarks_path";
const char *CONFIG_LOCATIONS[] = {
        "chromium",
        "google-chrome",
        "BraveSoftware",
        "brave-browser",
        "vivaldi"
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
    QString bookmarks_file_path;

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
        futureWatcher.setFuture(QtConcurrent::run(getBookmarks, bookmarks_file_path));

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
        if (!fileSystemWatcher.files().contains(bookmarks_file_path))
            if(!fileSystemWatcher.addPath(bookmarks_file_path))
                    WARN << bookmarks_file_path << "can not be watched. Changes in this path will not be noticed.";

        // Notification
        auto msg = QString("%1 bookmarks indexed.").arg(index.size());
        INFO << msg;
        emit q->statusInfo(msg);
    }


    static vector<shared_ptr<StandardIndexItem>> getBookmarks(const QString& filePath){

        vector<shared_ptr<StandardIndexItem>> bookmarkItems;

        QString icon = XDG::IconLookup::iconPath({"www", "web-browser", "emblem-web"});
        icon = icon.isEmpty() ? ":favicon" : icon;

        // For each bookmarks file
        QFile f(filePath);
        if (f.open(QIODevice::ReadOnly)) {

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

                auto item = makeStdIdxItem(
                    QString("%1.%2").arg(EXT_ID, guid), icon, title, url,
                    IdxStrList{{title, UINT_MAX},
                               {url, UINT_MAX/2}},
                    ActionList{makeUrlAction("Open URL", url),
                               makeClipAction("Copy URL to clipboard", url)}
                );
                bookmarkItems.emplace_back(item);
            }
            f.close();
        } else
            WARN << "Could not open Chrome bookmarks file:" << filePath;

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
    auto path = settings().value(CFG_BOOKMARKS_PATH);

    // If path is unset try to automatically set chromium/chrome paths
    if (!path.isNull())
        setPath(path.toString());
    else {
        QDir configDir = QDir(QStandardPaths::writableLocation(QStandardPaths::ConfigLocation));
        for (const char *configLocation : CONFIG_LOCATIONS){
            QString root = configDir.filePath(configLocation);
            QDirIterator it(root, {"Bookmarks"}, QDir::Files, QDirIterator::Subdirectories);
            while (it.hasNext()){
                setPath(it.next());
                goto found;
            }
        }
        // MessageBox?
    }
    found:

    // Update index when a "Bookmark" file changed
    connect(&d->fileSystemWatcher, &QFileSystemWatcher::fileChanged,
            this, [this](){ d->startIndexing(); });

    registerQueryHandler(this);
}


/** ***************************************************************************/
Chromium::Extension::~Extension() {}


/** ***************************************************************************/
QWidget *Chromium::Extension::widget(QWidget *parent) {
    if (d->widget.isNull()){
        d->widget = new QWidget(parent);

        Ui::ConfigWidget ui;
        ui.setupUi(d->widget);

        // Path
        ui.line_edit_path->setText(d->bookmarks_file_path);

        connect(ui.push_button_set_path, &QPushButton::clicked, [this](){
            auto path = QFileDialog::getOpenFileName(d->widget,
                                                     tr("Select Bookmarks file"),
                                                     d->bookmarks_file_path.isNull()
                                                        ? d->bookmarks_file_path : QDir::homePath(),
                                                     tr("Bookmarks (Bookmarks)"));
            if (!path.isNull())
                setPath(path);
        });

        // Fuzzy
        ui.checkBox_fuzzy->setChecked(fuzzy());
        connect(ui.checkBox_fuzzy, &QCheckBox::toggled, this, &Extension::setFuzzy);

        // Status bar
        ( d->futureWatcher.isRunning() )
            ? ui.label_statusbar->setText("Indexing bookmarks ...")
            : ui.label_statusbar->setText(QString("%1 bookmarks indexed.").arg(d->index.size()));
        connect(this, &Extension::statusInfo, ui.label_statusbar, &QLabel::setText);
    }
    return d->widget.data();
}


/** ***************************************************************************/
bool Chromium::Extension::fuzzy() const {
    return d->offlineIndex.fuzzy();
}


/** ***************************************************************************/
void Chromium::Extension::setFuzzy(bool b) {
    settings().setValue(CFG_FUZZY, b);
    d->offlineIndex.setFuzzy(b);
}

/** ***************************************************************************/
const QString &Chromium::Extension::path() const {
    return d->bookmarks_file_path;
}

void Chromium::Extension::setPath(const QString path) {
    settings().setValue(CFG_BOOKMARKS_PATH, path);
    d->bookmarks_file_path = path;
    d->startIndexing();
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

