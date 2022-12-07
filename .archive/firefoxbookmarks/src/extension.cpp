// Copyright (c) 2022 Manuel Schneider

#include <QApplication>
#include <QCheckBox>
#include <QClipboard>
#include <QComboBox>
#include <QDesktopServices>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QFileSystemWatcher>
#include <QFutureWatcher>
#include <QPointer>
#include <QProcess>
#include <QPushButton>
#include <QSettings>
#include <QSqlDatabase>
#include <QSqlDriver>
#include <QSqlError>
#include <QSqlQuery>
#include <QStandardPaths>
#include <QTemporaryFile>
#include <QThreadPool>
#include <QUrl>
#include <QtConcurrent>
#include <functional>
#include <map>
#include <sys/sendfile.h>
#include "albert/extension.h"
#include "albert/util/offlineindex.h"
#include "albert/util/standardactions.h"
#include "albert/util/standardindexitem.h"
#include "configwidget.h"
#include "extension.h"
#include "xdg/iconlookup.h"
Q_LOGGING_CATEGORY(qlc, "firefox")
#define DEBG qCDebug(qlc,).noquote()
#define INFO qCInfo(qlc,).noquote()
#define WARN qCWarning(qlc,).noquote()
#define CRIT qCCritical(qlc,).noquote()
using namespace std;
using namespace Core;

namespace {
const QString CFG_FIREFOX_HOME = "profilesIniPath";
const QString CFG_FIREFOX_EXE = "firefoxPath";
const QString CFG_PROFILE = "profile";
const QString CFG_FUZZY   = "fuzzy";
const bool    DEF_FUZZY   = false;
const QString CFG_USE_FIREFOX   = "openWithFirefox";
const bool    DEF_USE_FIREFOX   = false;
const uint    UPDATE_DELAY = 60000;
}



/** ***************************************************************************/
/** ***************************************************************************/
/** ***************************************************************************/
/** ***************************************************************************/
class FirefoxBookmarks::Private
{
public:
    Private(Extension *q) : q(q) {}

    Extension *q;

    bool openWithFirefox;
    QPointer<ConfigWidget> widget;
    QString firefoxExecutable;
    QString profilesIniPath;
    QString currentProfileId;
    QString dbPath;
    QFileSystemWatcher databaseWatcher;

    vector<shared_ptr<Core::StandardIndexItem>> index;
    Core::OfflineIndex offlineIndex;

    QTimer updateDelayTimer;
    void startIndexing();
    void finishIndexing();
    QFutureWatcher<vector<shared_ptr<Core::StandardIndexItem>>> futureWatcher;
    vector<shared_ptr<Core::StandardIndexItem>> indexFirefoxBookmarks() const;
};


/** ***************************************************************************/
void FirefoxBookmarks::Private::startIndexing() {

    // Never run concurrent
    if ( futureWatcher.future().isRunning() )
        return;

    // Run finishIndexing when the indexing thread finished
    futureWatcher.disconnect();
    QObject::connect(&futureWatcher, &QFutureWatcher<vector<shared_ptr<Core::StandardIndexItem>>>::finished,
                     bind(&Private::finishIndexing, this));

    // Run the indexer thread
    futureWatcher.setFuture(QtConcurrent::run(this, &Private::indexFirefoxBookmarks));

    // Notification
    INFO << "Start indexing Firefox bookmarks.";
    emit q->statusInfo("Indexing bookmarks ...");
}


/** ***************************************************************************/
void FirefoxBookmarks::Private::finishIndexing() {

    auto result = futureWatcher.future().result();

    if ( result.empty() )
        return;

    // Get the thread results
    index = std::move(result);

    // Rebuild the offline index
    offlineIndex.clear();
    for (const auto &item : index)
        offlineIndex.add(item);

    // Notification
    INFO <<  qPrintable(QString("Indexed %1 Firefox bookmarks.").arg(index.size()));
    emit q->statusInfo(QString("%1 bookmarks indexed.").arg(index.size()));
}



/** ***************************************************************************/
vector<shared_ptr<Core::StandardIndexItem>>
FirefoxBookmarks::Private::indexFirefoxBookmarks() const {
    // Build a new index
    vector<shared_ptr<StandardIndexItem>> bookmarks;

    {
        QTemporaryFile dbcopy;
        dbcopy.setAutoRemove(true);
        if (!dbcopy.open()) {
            WARN << qPrintable(QString("Could not open temporary file: %1").arg(dbcopy.errorString()));
            return vector<shared_ptr<Core::StandardIndexItem>>();
        }

        QFile places(dbPath);
        if (!places.open(QIODevice::ReadOnly)) {
            WARN << qPrintable(QString("Could not open places.sqlite file: %1").arg(dbcopy.errorString()));
            return vector<shared_ptr<Core::StandardIndexItem>>();
        }

        bool sendfile_worked = false;
        {
            int source = places.handle();
            int destination = dbcopy.handle();
            if (sendfile(destination, source, nullptr, places.size()) != -1) {
                sendfile_worked = true;
            } else {
                DEBG << "sendfile did not work, falling back to userland buffer";
            }
        }

        if (!sendfile_worked) {
            char buf[512];
            int read;
            do {
                read = places.read(buf, 512);
                if (read == -1) {
                    WARN << qPrintable(QString("Could not copy places.sqlite file: read failed: %1").arg(places.errorString()));
                    return vector<shared_ptr<Core::StandardIndexItem>>();
                }
                if (dbcopy.write(buf, read) == -1) {
                    WARN << qPrintable(QString("Could not copy places.sqlite file: write failed: %1").arg(dbcopy.errorString()));
                    return vector<shared_ptr<Core::StandardIndexItem>>();
                }
            } while (read > 0);
        }

        QSqlDatabase database = QSqlDatabase::addDatabase("QSQLITE", q->Core::Plugin::id());;
        database.setDatabaseName(dbcopy.fileName());

        if (!database.open()) {
            WARN << qPrintable(QString("Could not open Firefox database: %1").arg(database.databaseName()));
            return vector<shared_ptr<Core::StandardIndexItem>>();
        }

        QSqlQuery result(database);

        if ( !result.exec("SELECT bookmarks.guid, bookmarks.title, places.url "
                          "FROM moz_bookmarks bookmarks "
                          "JOIN moz_bookmarks parents ON bookmarks.parent = parents.id AND parents.parent <> 4  "
                          "JOIN moz_places places ON bookmarks.fk = places.id "
                          "WHERE NOT hidden") ) {  // Those with place:... will not work with xdg-open
            WARN << qPrintable(QString("Querying Firefox bookmarks failed: %1").arg(result.lastError().text()));
            return vector<shared_ptr<Core::StandardIndexItem>>();
        }

        // Find an appropriate icon
        QString icon = XDG::IconLookup::iconPath({"www", "web-browser", "emblem-web"});
        icon = icon.isEmpty() ? ":favicon" : icon;

        while (result.next()) {

            // Url will be used more often
            QString urlstr = result.value(2).toString();

            // Add actions
            ActionList actions;
            actions.push_back(makeProcAction("Open URL in Firefox",
                                             QStringList({firefoxExecutable, urlstr})));
            actions.push_back(makeProcAction("Open URL in new Firefox window",
                                             QStringList({firefoxExecutable, "--new-window", urlstr})));
            actions.insert(openWithFirefox ? actions.end() : actions.begin(),
                           makeUrlAction("Open URL in your default browser", urlstr));
            actions.push_back(makeClipAction("Copy url to clipboard", urlstr));

            // Create item
            QUrl url(urlstr);
            QString host = url.host();
            shared_ptr<StandardIndexItem> item = makeStdIdxItem(result.value(0).toString(),
                icon, result.value(1).toString(), urlstr,
                IdxStrList {
                    {result.value(1).toString(), UINT_MAX},
                    {host.left(host.size()-url.topLevelDomain().size()), UINT_MAX/2},
                    {result.value(2).toString(), UINT_MAX/4},
                },
                actions);

            bookmarks.push_back(move(item));
        }

        database.close();
    }

    QSqlDatabase::removeDatabase(q->Core::Plugin::id());

    return bookmarks;
}



/** ***************************************************************************/
/** ***************************************************************************/
/** ***************************************************************************/
/** ***************************************************************************/
FirefoxBookmarks::Extension::Extension()
    : Core::Extension("org.albert.extension.firefoxbookmarks"),
      Core::QueryHandler(Core::Plugin::id()),
      d(new Private(this)){

    registerQueryHandler(this);

    // Add a sqlite database connection for this extension, check requirements
    {
        QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", Core::Plugin::id());
        if ( !db.isValid() )
            throw "Invalid Database.";
        if (!db.driver()->hasFeature(QSqlDriver::Transactions))
            throw "DB Driver does not support transactions.";
    }
    QSqlDatabase::removeDatabase(Core::Plugin::id());

    // Find firefox executable
    d->firefoxExecutable = settings().value(CFG_FIREFOX_EXE, "").toString();
    if (d->firefoxExecutable.isEmpty()) {
        d->firefoxExecutable = QStandardPaths::findExecutable("firefox");
    }
    if (d->firefoxExecutable.isEmpty())
        throw "Firefox executable not found.";
    settings().setValue(CFG_FIREFOX_EXE, d->firefoxExecutable);

    // Locate profiles ini
    d->profilesIniPath = settings().value(CFG_FIREFOX_HOME, "").toString();
    if (d->profilesIniPath.isEmpty()) {
        d->profilesIniPath = QStandardPaths::locate(QStandardPaths::HomeLocation,
                                                     ".mozilla/firefox/profiles.ini",
                                                     QStandardPaths::LocateFile);
        if (d->profilesIniPath.isEmpty()) // Try a windowsy approach
            d->profilesIniPath = QStandardPaths::locate(QStandardPaths::DataLocation,
                                                         "Mozilla/firefox/profiles.ini",
                                                         QStandardPaths::LocateFile);
    }
    if (d->profilesIniPath.isEmpty())
        throw "Could not locate profiles.ini.";
    settings().setValue(CFG_FIREFOX_HOME, d->profilesIniPath);

    // Load the settings
    d->currentProfileId = settings().value(CFG_PROFILE).toString();
    d->offlineIndex.setFuzzy(settings().value(CFG_FUZZY, DEF_FUZZY).toBool());
    d->openWithFirefox = settings().value(CFG_USE_FIREFOX, DEF_USE_FIREFOX).toBool();

    // If the id does not exist find a proper default
    QSettings profilesIni(d->profilesIniPath, QSettings::IniFormat);
    QString pathkey = QString("%1/Path").arg(d->currentProfileId);
    if ( !profilesIni.contains(pathkey) ){

        d->currentProfileId = QString();

        QStringList ids = profilesIni.childGroups();
        if ( ids.isEmpty() )
            WARN << "No Firefox profiles found.";
        else {

            // Use the last used profile
            if ( d->currentProfileId.isNull() ) {
                for (QString &id : ids) {
                    profilesIni.beginGroup(id);
                    if ( profilesIni.contains("Path")
                         && profilesIni.contains("Default")
                         && profilesIni.value("Default").toInt() > 0 )  {
                        d->currentProfileId = id;
                    }
                    profilesIni.endGroup();
                }
            }

            // Use the default profile
            if ( d->currentProfileId.isNull() && ids.contains("default")) {
                d->currentProfileId = "default";
            }

            if ( d->currentProfileId.isNull() ) {
                // Use the first
                d->currentProfileId = ids[0];
            }
        }
    }

    // Set the profile
    setProfile(d->currentProfileId);

    // Delay the indexing to avoid excessice resource consumption
    d->updateDelayTimer.setInterval(UPDATE_DELAY);
    d->updateDelayTimer.setSingleShot(true);

    // If the database changed, trigger the update delay
    connect(&d->databaseWatcher, &QFileSystemWatcher::fileChanged,
            &d->updateDelayTimer, static_cast<void(QTimer::*)()>(&QTimer::start));

    // If the update delay passed, update the index
    connect(&d->updateDelayTimer, &QTimer::timeout,
            bind(&Private::startIndexing, d.get()));
}



/** ***************************************************************************/
FirefoxBookmarks::Extension::~Extension() {

}



/** ***************************************************************************/
QWidget *FirefoxBookmarks::Extension::widget(QWidget *parent) {
    if (d->widget.isNull()) {
        d->widget = new ConfigWidget(parent);

        // Get the profiles keys
        QSettings profilesIni(d->profilesIniPath, QSettings::IniFormat);
        QStringList groups = profilesIni.childGroups();

        // Extract all profiles and names and put it in the checkbox
        QComboBox *cmb = d->widget->ui.comboBox;
        for (QString &profileId : groups) {
            profilesIni.beginGroup(profileId);

            // Use name if available else id
            if ( profilesIni.contains("Name") )
                cmb->addItem( QString("%1 (%2)").arg(profilesIni.value("Name").toString(), profileId), profileId);
            else {
                cmb->addItem(profileId, profileId);
                WARN << qPrintable(QString("Firefox profile '%1' does not contain a name.").arg(profileId));
            }

            // If the profileId match set the current item of the checkbox
            if (profileId == d->currentProfileId)
                cmb->setCurrentIndex(cmb->count() - 1);

            profilesIni.endGroup();
        }

        connect(cmb, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
                this, static_cast<void(Extension::*)(int)>(&Extension::setProfile));

        // profiles.ini Path Label
        d->widget->ui.label_profiles->setText("profiles.ini Path: " + d->profilesIniPath);

        // discovered ff executable
        d->widget->ui.label_exe->setText("Detected Firefox executable: " + d->firefoxExecutable);

        // Fuzzy
        QCheckBox *ckb = d->widget->ui.fuzzy;
        ckb->setChecked(d->offlineIndex.fuzzy());
        connect(ckb, &QCheckBox::clicked, this, &Extension::changeFuzzyness);

        // Which app to use
        ckb = d->widget->ui.openWithFirefox;
        ckb->setChecked(d->openWithFirefox);
        connect(ckb, &QCheckBox::clicked, this, &Extension::changeOpenPolicy);

        // Status bar
        ( d->futureWatcher.isRunning() )
            ? d->widget->ui.label_statusbar->setText("Indexing bookmarks ...")
            : d->widget->ui.label_statusbar->setText(QString("%1 bookmarks indexed.").arg(d->index.size()));
        connect(this, &Extension::statusInfo, d->widget->ui.label_statusbar, &QLabel::setText);

	// Rescan button
	QPushButton *btn = d->widget->ui.pushButton_update;
        connect(btn, &QPushButton::clicked, bind(&Private::startIndexing, d.get()));

    }
    return d->widget;
}



/** ***************************************************************************/
void FirefoxBookmarks::Extension::handleQuery(Core::Query *query) const {

    const vector<shared_ptr<Core::IndexableItem>> &indexables = d->offlineIndex.search(query->string());

    vector<pair<shared_ptr<Core::Item>,uint>> results;
    for (const shared_ptr<Core::IndexableItem> &item : indexables)
        results.emplace_back(static_pointer_cast<Core::StandardIndexItem>(item), 0);

    query->addMatches(make_move_iterator(results.begin()),
                      make_move_iterator(results.end()));
}



/** ***************************************************************************/
void FirefoxBookmarks::Extension::setProfile(int index) {

    QComboBox *cmb = d->widget->ui.comboBox;
    QVariant profileId = cmb->itemData(index);

    setProfile(profileId.toString());
}


/** ***************************************************************************/
void FirefoxBookmarks::Extension::setProfile(const QString& profile) {
    d->currentProfileId = profile;

    QSettings profilesIni(d->profilesIniPath, QSettings::IniFormat);

    // Check if profile id is in profiles file
    if ( !profilesIni.childGroups().contains(d->currentProfileId) ){
        WARN << qPrintable(QString("Firefox user profile '%2' not found.").arg(d->currentProfileId));
        return;
    }

    // Enter the group
    profilesIni.beginGroup(d->currentProfileId);

    // Check if the profile contains a path key
    if ( !profilesIni.contains("Path") ){
        WARN << qPrintable(QString("Firefox profile '%2' does not contain a path.").arg(d->currentProfileId));
        return;
    }

    // Get the correct absolute profile path
    QString profilePath = ( profilesIni.contains("IsRelative") && profilesIni.value("IsRelative").toBool())
            ? QFileInfo(d->profilesIniPath).dir().absoluteFilePath(profilesIni.value("Path").toString())
            : profilesIni.value("Path").toString();

    // Build the database path
    QString dbPath = QString("%1/places.sqlite").arg(profilePath);

    // Set the databases path
    d->dbPath = dbPath;

    // Set a file system watcher on the database monitoring changes
    if (!d->databaseWatcher.files().isEmpty())
        d->databaseWatcher.removePaths(d->databaseWatcher.files());
    d->databaseWatcher.addPath(dbPath);

    d->startIndexing();

    settings().setValue(CFG_PROFILE, d->currentProfileId);
}



/** ***************************************************************************/
void FirefoxBookmarks::Extension::changeFuzzyness(bool fuzzy) {
    d->offlineIndex.setFuzzy(fuzzy);
    settings().setValue(CFG_FUZZY, fuzzy);
}



/** ***************************************************************************/
void FirefoxBookmarks::Extension::changeOpenPolicy(bool useFirefox) {
    d->openWithFirefox = useFirefox;
    settings().setValue(CFG_USE_FIREFOX, useFirefox);
    d->startIndexing();
}
