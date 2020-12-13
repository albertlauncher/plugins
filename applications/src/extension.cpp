// Copyright (C) 2014-2020 Manuel Schneider

#include <QApplication>
#include <QDir>
#include <QDirIterator>
#include <QFile>
#include <QFileSystemWatcher>
#include <QPointer>
#include <QProcess>
#include <QRegularExpression>
#include <QSettings>
#include <QStandardPaths>
#include <QThread>
#include <QTimer>
#include <QtConcurrent>
#include <algorithm>
#include <functional>
#include <map>
#include <memory>
#include <vector>
#include "albert/queryhandler.h"
#include "albert/util/offlineindex.h"
#include "albert/util/standardactions.h"
#include "albert/util/standardindexitem.h"
#include "configwidget.h"
#include "extension.h"
#include "xdg/iconlookup.h"
Q_LOGGING_CATEGORY(qlc, "apps")
#define DEBG qCDebug(qlc,).noquote()
#define INFO qCInfo(qlc,).noquote()
#define WARN qCWarning(qlc,).noquote()
#define CRIT qCCritical(qlc,).noquote()
using namespace Core;
using namespace std;

namespace {

const char* CFG_FUZZY                = "fuzzy";
const bool  DEF_FUZZY                = false;
const char* CFG_IGNORESHOWINKEYS     = "ignore_show_in_keys";
const bool  DEF_IGNORESHOWINKEYS     = false;
const char* CFG_USEKEYWORDS          = "use_keywords";
const bool  DEF_USEKEYWORDS          = false;
const char* CFG_USEGENERICNAME       = "use_generic_name";
const bool  DEF_USEGENERICNAME       = false;
const char* CFG_USENONLOCALIZEDNAME  = "use_non_localized_name";
const bool  DEF_USENONLOCALIZEDNAME  = false;

/******************************************************************************/
QString fieldCodesExpanded(const QString & exec, const QString & name, const QString & icon, const QString & de_path) {
    /*
     * https://specifications.freedesktop.org/desktop-entry-spec/1.5/ar01s07.html
     *
     * Code	Description
     * %% : '%'
     * %f : A single file name (including the path), even if multiple files are selected. The system reading the desktop entry should recognize that the program in question cannot handle multiple file arguments, and it should should probably spawn and execute multiple copies of a program for each selected file if the program is not able to handle additional file arguments. If files are not on the local file system (i.e. are on HTTP or FTP locations), the files will be copied to the local file system and %f will be expanded to point at the temporary file. Used for programs that do not understand the URL syntax.
     * %F : A list of files. Use for apps that can open several local files at once. Each file is passed as a separate argument to the executable program.
     * %u : A single URL. Local files may either be passed as file: URLs or as file path.
     * %U : A list of URLs. Each URL is passed as a separate argument to the executable program. Local files may either be passed as file: URLs or as file path.
     * %i : The Icon key of the desktop entry expanded as two arguments, first --icon and then the value of the Icon key. Should not expand to any arguments if the Icon key is empty or missing.
     * %c : The translated name of the application as listed in the appropriate Name key in the desktop entry.
     * %k : The location of the desktop file as either a URI (if for example gotten from the vfolder system) or a local filename or empty if no location is known.
     * Deprecated: %v %m %d %D %n %N
     */
    QString commandLine;
    for (auto it = exec.cbegin(); it != exec.end(); ++it) {
        if (*it == '%'){
            ++it;
            if (it == exec.end()) {
                break;
            } else if (*it=='%') {
                commandLine.push_back("%");
            } else if (*it=='f') {  // Unhandled atm
            } else if (*it=='F') {  // Unhandled atm
            } else if (*it=='u') {  // Unhandled atm
            } else if (*it=='U') {  // Unhandled atm
            } else if (*it=='i' && !icon.isNull()) {
                commandLine.push_back(QString("--icon %1").arg(icon));
            } else if (*it=='c') {
                commandLine.push_back(name);
            } else if (*it=='k') {
                commandLine.push_back(de_path);
            } else if (*it=='v' || *it=='m' || *it=='d' || *it=='D' || *it=='n' || *it=='N') { /*Skipping deprecated field codes*/
            } else {
                qWarning() << "Ignoring invalid field code: " << *it;
            }
        } else
            commandLine.push_back(*it);
    }
    return commandLine;
}
/******************************************************************************/
QString xdgStringEscape(const QString & unescaped) {
    /*
     * The escape sequences \s, \n, \t, \r, and \\ are supported for values of
     * type string and localestring, meaning ASCII space, newline, tab, carriage
     * return, and backslash, respectively.
     *
     * http://standards.freedesktop.org/desktop-entry-spec/latest/ar01s03.html
     */
    QString result;
    QString::const_iterator it = unescaped.begin();
    while (it != unescaped.end()) {
        if (*it == '\\'){
            ++it;
            if (it == unescaped.end())
                break;
            else if (*it=='s')
                result.append(' ');
            else if (*it=='n')
                result.append('\n');
            else if (*it=='t')
                result.append('\t');
            else if (*it=='r')
                result.append('\r');
            else if (*it=='\\')
                result.append('\\');
        }
        else
            result.append(*it);
        ++it;
    }
    return result;
}

/******************************************************************************/
QString getLocalizedKey(const QString &key, const map<QString,QString> &entries, const QLocale &loc) {
    map<QString,QString>::const_iterator it;
    if ( (it = entries.find(QString("%1[%2]").arg(key, loc.name()))) != entries.end()
         || (it = entries.find(QString("%1[%2]").arg(key, loc.name().left(2)))) != entries.end()
         || (it = entries.find(key)) != entries.end())
        return it->second;
    return QString();
}

}



/** ***************************************************************************/
/** ***************************************************************************/
/** ***************************************************************************/
/** ***************************************************************************/
class Applications::Private
{
public:
    Private(Extension *q) : q(q) {}

    Extension *q;

    QPointer<ConfigWidget> widget;
    QFileSystemWatcher watcher;

    QStringList indexedDirs;
    vector<shared_ptr<Core::StandardIndexItem>> index;
    OfflineIndex offlineIndex;

    QFutureWatcher<vector<shared_ptr<Core::StandardIndexItem>>> futureWatcher;
    bool rerun = false;
    bool ignoreShowInKeys;
    bool useKeywords;
    bool useGenericName;
    bool useNonLocalizedName;

    void finishIndexing();
    void startIndexing();
    vector<shared_ptr<Core::StandardIndexItem>> indexApplications() const;
};



/** ***************************************************************************/
void Applications::Private::startIndexing() {

    // Never run concurrent
    if ( futureWatcher.future().isRunning() ) {
        rerun = true;
        return;
    }

    // Run finishIndexing when the indexing thread finished
    futureWatcher.disconnect();
    QObject::connect(&futureWatcher, &QFutureWatcher<vector<shared_ptr<Core::StandardIndexItem>>>::finished,
                     bind(&Private::finishIndexing, this));

    // Run the indexer thread
    futureWatcher.setFuture(QtConcurrent::run(this, &Private::indexApplications));

    // Notification
    INFO << "Start indexing applications.";
    emit q->statusInfo("Indexing applications ...");
}



/** ***************************************************************************/
void Applications::Private::finishIndexing() {

    // Get the thread results
    index = futureWatcher.future().result();

    // Rebuild the offline index
    offlineIndex.clear();
    for (const auto &item : index)
        offlineIndex.add(item);

    // Finally update the watches (maybe folders changed)
    if (!watcher.directories().isEmpty())
        watcher.removePaths(watcher.directories());
    QStringList xdgAppDirs = QStandardPaths::standardLocations(QStandardPaths::ApplicationsLocation);
    for (const QString &path : xdgAppDirs) {
        if (QFile::exists(path)) {
            watcher.addPath(path);
            QDirIterator dit(path, QDir::Dirs|QDir::NoDotAndDotDot);
            while (dit.hasNext())
                watcher.addPath(dit.next());
        }
    }

    // Notification
    INFO << QString("Indexed %1 applications.").arg(index.size());
    emit q->statusInfo(QString("%1 applications indexed.").arg(index.size()));

    if ( rerun ) {
        startIndexing();
        rerun = false;
    }
}

/** ***************************************************************************/
vector<shared_ptr<StandardIndexItem>> Applications::Private::indexApplications() const {

    // Get a new index [O(n)]
    vector<shared_ptr<StandardIndexItem>> desktopEntries;
    QStringList xdg_current_desktop = QString(getenv("XDG_CURRENT_DESKTOP")).split(':',QString::SkipEmptyParts);
    QLocale loc;
    QStringList xdgAppDirs = QStandardPaths::standardLocations(QStandardPaths::ApplicationsLocation);
    xdgAppDirs.append(QStandardPaths::standardLocations(QStandardPaths::DesktopLocation));
	
    /*
     * Create a list of desktop files to index (unique ids)
     * To determine the ID of a desktop file, make its full path relative to
     * the $XDG_DATA_DIRS component in which the desktop file is installed,
     * remove the "applications/" prefix, and turn '/' into '-'.
     */

    map<QString /*desktop file id*/, QString /*path*/> desktopFiles;
    for ( const QString &dir : xdgAppDirs ) {
        QDirIterator fIt(dir, QStringList("*.desktop"), QDir::Files,
                         QDirIterator::Subdirectories|QDirIterator::FollowSymlinks);
        while (!fIt.next().isEmpty()) {
            QString desktopFileId = fIt.filePath().remove(QRegularExpression("^.*applications/")).replace("/","-");
            const auto &pair = desktopFiles.emplace(desktopFileId, fIt.filePath());
            if (!pair.second)
                INFO << QString("Desktop file skipped: '%1' overwritten in '%2'").arg(fIt.filePath(), desktopFiles[desktopFileId]);
        }
    }

    // Iterate over all desktop files
    for (const auto &id_path_pair : desktopFiles) {

        const QString &id = id_path_pair.first;
        const QString &path = id_path_pair.second;

        DEBG << "Indexing desktop file:" << id;

        map<QString,map<QString,QString>> sectionMap;
        map<QString,map<QString,QString>>::iterator sectionIterator;

        /*
         * Get the data from the desktop file
         */

        // Read the file into a map
        {
            QFile file(path);
            if (!file.open(QIODevice::ReadOnly| QIODevice::Text)) continue;
            QTextStream stream(&file);
            QString currentGroup;
            for (QString line=stream.readLine(); !line.isNull(); line=stream.readLine()) {
                line = line.trimmed();
                if (line.startsWith('#') || line.isEmpty())
                    continue;
                if (line.startsWith("[")){
                    currentGroup = line.mid(1,line.size()-2).trimmed();
                    continue;
                }
                sectionMap[currentGroup].emplace(line.section('=', 0,0).trimmed(),
                                                 line.section('=', 1, -1).trimmed());
            }
            file.close();
        }


        // Skip if there is no "Desktop Entry" section
        if ((sectionIterator = sectionMap.find("Desktop Entry")) == sectionMap.end())
            continue;

        map<QString,QString> const &entryMap = sectionIterator->second;
        map<QString,QString>::const_iterator entryIterator;

        // Skip, if type is not found or not application
        if ((entryIterator = entryMap.find("Type")) == entryMap.end() ||
                entryIterator->second != "Application")
            continue;

        // Skip, if this desktop entry must not be shown
        if ((entryIterator = entryMap.find("NoDisplay")) != entryMap.end()
                && entryIterator->second == "true")
            continue;

        if (!ignoreShowInKeys) {
            // Skip if the current desktop environment is specified in "NotShowIn"
            if ((entryIterator = entryMap.find("NotShowIn")) != entryMap.end())
                for (const QString &str : entryIterator->second.split(';',QString::SkipEmptyParts))
                    if (xdg_current_desktop.contains(str))
                        continue;

            // Skip if the current desktop environment is not specified in "OnlyShowIn"
            if ((entryIterator = entryMap.find("OnlyShowIn")) != entryMap.end()) {
                bool found = false;
                for (const QString &str : entryIterator->second.split(';',QString::SkipEmptyParts))
                    if (xdg_current_desktop.contains(str)){
                        found = true;
                        break;
                    }
                if (!found)
                    continue;
            }
        }

        bool term;
        QString name;
        QString nonLocalizedName;
        QString genericName;
        QString comment;
        QString icon;
        QString exec;
        QString workingDir;
        QStringList keywords;
        QStringList actionIdentifiers;

        // Try to get the localized name, skip if empty
        name = xdgStringEscape(getLocalizedKey("Name", entryMap, loc));
        if (name.isNull())
            continue;

        // Try to get the exec key, skip if not existant
        if ((entryIterator = entryMap.find("Exec")) != entryMap.end())
            exec = xdgStringEscape(entryIterator->second);
        else
            continue;

        // Try to get the localized icon, skip if empty
        icon = xdgStringEscape(getLocalizedKey("Icon", entryMap, loc));

        // Check if this is a terminal app
        term = (entryIterator = entryMap.find("Terminal")) != entryMap.end()
                && entryIterator->second=="true";

        // Try to get the localized genericName
        genericName = xdgStringEscape(getLocalizedKey("GenericName", entryMap, loc));

        // Try to get the non-localized name
        if ((entryIterator = entryMap.find("Name")) != entryMap.end())
            nonLocalizedName = xdgStringEscape(entryIterator->second);

        // Try to get the localized comment
        comment = xdgStringEscape(getLocalizedKey("Comment", entryMap, loc));

        // Try to get the keywords
        keywords = xdgStringEscape(getLocalizedKey("Keywords", entryMap, loc)).split(';',QString::SkipEmptyParts);

        // Try to get the workindir
        if ((entryIterator = entryMap.find("Path")) != entryMap.end())
            workingDir = xdgStringEscape(entryIterator->second);

        // Try to get the keywords
        if ((entryIterator = entryMap.find("Actions")) != entryMap.end())
            actionIdentifiers = xdgStringEscape(entryIterator->second).split(';',QString::SkipEmptyParts);

//            // Try to get the mimetypes
//            if ((valueIterator = entryMap.find("MimeType")) != entryMap.end())
//                keywords = xdgStringEscape(valueIterator->second).split(';',QString::SkipEmptyParts);


        /*
         * Build the item
         */

        // Field code expandesd commandline
        QString commandLine = fieldCodesExpanded(exec, name, icon, path);

        // Icon path
        QString icon_path = XDG::IconLookup::iconPath({icon, "application-x-executable", "exec"});
        if (icon_path.isNull())
            icon_path = ":application-x-executable";

        // Description
        QString subtext;
        if (!comment.isEmpty())
            subtext = comment;
        else if(useGenericName && !genericName.isEmpty())
            subtext = genericName;
        else if(useNonLocalizedName && !nonLocalizedName.isEmpty())
            subtext = nonLocalizedName;
        else
            subtext = commandLine;

        // Finally we got everything, build the item
        auto item = make_shared<StandardIndexItem>(id, icon_path, name, subtext);

        // Set index strings
        vector<IndexableItem::IndexString> indexStrings;
        indexStrings.emplace_back(name, UINT_MAX);

        QStringList excludes = {
            "java ",
            "ruby ",
            "python ",
            "perl ",
            "bash ",
            "sh ",
            "dbus-send ",
            "/"
        };
        if (none_of(excludes.begin(), excludes.end(), [&exec](const QString &str){ return exec.startsWith(str); }))
            indexStrings.emplace_back(exec.section(QChar(QChar::Space), 0, 0, QString::SectionSkipEmpty), UINT_MAX);

        if (useKeywords)
            for (auto & kw : keywords)
                indexStrings.emplace_back(kw, UINT_MAX/2);

        if (useGenericName && !genericName.isEmpty())
            indexStrings.emplace_back(genericName, UINT_MAX/2);

        if (useNonLocalizedName && !nonLocalizedName.isEmpty())
            indexStrings.emplace_back(nonLocalizedName, UINT_MAX/2);

        item->setIndexKeywords(move(indexStrings));


        /*
         * Build actions
         */

        // Default and root action
        if (term)
            item->addAction(make_shared<ShTermAction>("Run", commandLine, ShTermAction::CloseOnExit, workingDir));
        else
            item->addAction(make_shared<ProcAction>("Run", QStringList() << "sh" << "-c" << commandLine, workingDir));

        // Desktop Actions
        for (const QString &actionIdentifier: actionIdentifiers){

            // Get iterator to action section
            if ((sectionIterator = sectionMap.find(QString("Desktop Action %1").arg(actionIdentifier))) == sectionMap.end())
                continue;
            map<QString,QString> &valueMap = sectionIterator->second;

            // Try to get the localized action name
            QString actionName = xdgStringEscape(getLocalizedKey("Name", valueMap, loc));
            if (actionName.isNull())
                continue;

            // Get action command
            if ((entryIterator = valueMap.find("Exec")) == valueMap.end())
                continue;

            // Unquote arguments and expand field codes
            commandLine = fieldCodesExpanded(entryIterator->second, icon, name, path);
            if (term)
                item->addAction(make_shared<ShTermAction>(actionName, commandLine, ShTermAction::CloseOnExit, workingDir));
            else
                item->addAction(make_shared<ProcAction>(actionName, QStringList() << "sh" << "-c" << commandLine, workingDir));

        }


        /*
         * Add item
         */

        desktopEntries.push_back(move(item));
    }
    return desktopEntries;
}


/** ***************************************************************************/
/** ***************************************************************************/
/** ***************************************************************************/
/** ***************************************************************************/
Applications::Extension::Extension()
    : Core::Extension("org.albert.extension.applications"),
      Core::QueryHandler(Core::Plugin::id()),
      d(new Private(this)) {

    registerQueryHandler(this);

    qunsetenv("DESKTOP_AUTOSTART_ID");


    d->indexedDirs = QStandardPaths::standardLocations(QStandardPaths::ApplicationsLocation)
                        << QStandardPaths::standardLocations(QStandardPaths::DesktopLocation);

    // Load settings
    d->offlineIndex.setFuzzy(settings().value(CFG_FUZZY, DEF_FUZZY).toBool());
    d->ignoreShowInKeys = settings().value(CFG_IGNORESHOWINKEYS, DEF_IGNORESHOWINKEYS).toBool();
    d->useGenericName = settings().value(CFG_USEGENERICNAME, DEF_USEGENERICNAME).toBool();
    d->useNonLocalizedName = settings().value(CFG_USENONLOCALIZEDNAME, DEF_USENONLOCALIZEDNAME).toBool();
    d->useKeywords = settings().value(CFG_USEKEYWORDS, DEF_USEKEYWORDS).toBool();

    // If the filesystem changed, trigger the scan
    connect(&d->watcher, &QFileSystemWatcher::directoryChanged,
            bind(&Private::startIndexing, d.get()));

    // Trigger initial update
    updateIndex();
}



/** ***************************************************************************/
Applications::Extension::~Extension() {
    d->futureWatcher.waitForFinished();
}



/** ***************************************************************************/
QWidget *Applications::Extension::widget(QWidget *parent) {
    if (d->widget.isNull()) {
        d->widget = new ConfigWidget(parent);

        // Show the app dirs in the label
        d->widget->ui.label->setText(d->widget->ui.label->text().replace("__XDG_DATA_DIRS__", d->indexedDirs.join(", ")));

        // Fuzzy
        d->widget->ui.checkBox_fuzzy->setChecked(d->offlineIndex.fuzzy());
        connect(d->widget->ui.checkBox_fuzzy, &QCheckBox::toggled,
                 this, &Extension::setFuzzy);

        // Use keywords
        d->widget->ui.checkBox_useKeywords->setChecked(d->useKeywords);
        connect(d->widget->ui.checkBox_useKeywords, &QCheckBox::toggled,
                this, [this](bool checked){
            settings().setValue(CFG_USEKEYWORDS, checked);
            d->useKeywords = checked;
            d->startIndexing();
        });

        // Use generic name
        d->widget->ui.checkBox_useGenericName->setChecked(d->useGenericName);
        connect(d->widget->ui.checkBox_useGenericName, &QCheckBox::toggled,
                this, [this](bool checked){
            settings().setValue(CFG_USEGENERICNAME, checked);
            d->useGenericName = checked;
            d->startIndexing();
        });

        // Use non-localized name
        d->widget->ui.checkBox_useNonLocalizedName->setChecked(d->useNonLocalizedName);
        connect(d->widget->ui.checkBox_useNonLocalizedName, &QCheckBox::toggled,
                this, [this](bool checked){
            settings().setValue(CFG_USENONLOCALIZEDNAME, checked);
            d->useNonLocalizedName = checked;
            d->startIndexing();
        });

        // Ignore onlyshowin notshowin keys
        d->widget->ui.checkBox_ignoreShowInKeys->setChecked(d->ignoreShowInKeys);
        connect(d->widget->ui.checkBox_ignoreShowInKeys, &QCheckBox::toggled,
                this, [this](bool checked){
            settings().setValue(CFG_IGNORESHOWINKEYS, checked);
            d->ignoreShowInKeys = checked;
            d->startIndexing();
        });

        // Status bar
        ( d->futureWatcher.isRunning() )
            ? d->widget->ui.label_statusbar->setText("Indexing applications ...")
            : d->widget->ui.label_statusbar->setText(QString("%1 applications indexed.").arg(d->index.size()));
        connect(this, &Extension::statusInfo, d->widget->ui.label_statusbar, &QLabel::setText);
    }
    return d->widget;
}



/** ***************************************************************************/
void Applications::Extension::handleQuery(Core::Query * query) const {

    const vector<shared_ptr<Core::IndexableItem>> &indexables = d->offlineIndex.search(query->string());

    vector<pair<shared_ptr<Core::Item>,uint>> results;
    for (const shared_ptr<Core::IndexableItem> &item : indexables)
        results.emplace_back(static_pointer_cast<Core::StandardIndexItem>(item), 1);

    query->addMatches(make_move_iterator(results.begin()),
                      make_move_iterator(results.end()));
}



/** ***************************************************************************/
bool Applications::Extension::fuzzy() {
    return d->offlineIndex.fuzzy();
}



/** ***************************************************************************/
void Applications::Extension::setFuzzy(bool b) {
    settings().setValue(CFG_FUZZY, b);
    d->offlineIndex.setFuzzy(b);
}



/** ***************************************************************************/
void Applications::Extension::updateIndex() {
    d->startIndexing();
}
