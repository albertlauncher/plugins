// Copyright (C) 2014-2017 Manuel Schneider

#include <QApplication>
#include <QDir>
#include <QDirIterator>
#include <QDebug>
#include <QFile>
#include <QFileSystemWatcher>
#include <QPointer>
#include <QProcess>
#include <QRegularExpression>
#include <QSettings>
#include <QStandardPaths>
#include <QtConcurrent>
#include <QTimer>
#include <QThread>
#include <algorithm>
#include <functional>
#include <map>
#include <memory>
#include <vector>
#include "configwidget.h"
#include "extension.h"
#include "core/queryhandler.h"
#include "util/offlineindex.h"
#include "util/standardactions.h"
#include "util/standardindexitem.h"
#include "util/shutil.h"
#include "xdg/iconlookup.h"
using namespace Core;
using namespace std;

extern QString terminalCommand;


namespace {

const char* CFG_FUZZY            = "fuzzy";
const bool  DEF_FUZZY            = false;
const char* CFG_IGNORESHOWINKEYS = "ignore_show_in_keys";
const bool  DEF_IGNORESHOWINKEYS = false;
const char* CFG_USEKEYWORDS      = "use_keywords";
const bool  DEF_USEKEYWORDS      = false;

/******************************************************************************/
QStringList expandedFieldCodes(const QStringList & unexpandedFields,
                               const QString & icon,
                               const QString & name,
                               const QString & path) {
    /*
     * A number of special field codes have been defined which will be expanded
     * by the file manager or program launcher when encountered in the command
     * line. Field codes consist of the percentage character ("%") followed by
     * an alpha character. Literal percentage characters must be escaped as %%.
     * Deprecated field codes should be removed from the command line and
     * ignored. Field codes are expanded only once, the string that is used to
     * replace the field code should not be checked for field codes itself.
     *
     * http://standards.freedesktop.org/desktop-entry-spec/latest/ar01s06.html
     */
    QStringList expandedFields;

    for (const QString & field : unexpandedFields){

        if (field == "%i" && !icon.isEmpty()) {
            expandedFields.push_back("--icon");
            expandedFields.push_back(icon);
        }

        QString tmpstr;
        QString::const_iterator it = field.begin();

        while (it != field.end()) {
            if (*it == '%'){
                ++it;
                if (it == field.end())
                    break;
                else if (*it=='%')
                    tmpstr.push_back("%");
                else if (*it=='c')
                    tmpstr.push_back(name);
                else if (*it=='k')
                    tmpstr.push_back(path);
                // TODO Unhandled f F u U
            }
            else
                tmpstr.push_back(*it);
            ++it;
        }
        if (!tmpstr.isEmpty())
            expandedFields.push_back(std::move(tmpstr));
    }
    return expandedFields;
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
    QString graphicalSudoPath;

    vector<shared_ptr<Core::StandardIndexItem>> index;
    OfflineIndex offlineIndex;

    QFutureWatcher<vector<shared_ptr<Core::StandardIndexItem>>> futureWatcher;
    bool rerun = false;
    bool ignoreShowInKeys;
    bool useKeywords;

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
                     std::bind(&Private::finishIndexing, this));

    // Run the indexer thread
    futureWatcher.setFuture(QtConcurrent::run(this, &Private::indexApplications));

    // Notification
    qInfo() << "Start indexing applications.";
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
    qInfo() << qPrintable(QString("Indexed %1 applications.").arg(index.size()));
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

    // Iterate over all desktop files
    for ( const QString &dir : xdgAppDirs ) {
        QDirIterator fIt(dir, QStringList("*.desktop"), QDir::Files,
                         QDirIterator::Subdirectories|QDirIterator::FollowSymlinks);
        while (fIt.hasNext()) {
            qDebug() << "Indexing desktop file:" << fIt.next();

            map<QString,map<QString,QString>> sectionMap;
            map<QString,map<QString,QString>>::iterator sectionIterator;

            // Build the id

            QString id;

            // If file is in standard path use standard method else filename to build id
            if ( std::any_of(xdgAppDirs.begin(), xdgAppDirs.end(),
                             [&fIt](const QString &dir){ return fIt.filePath().startsWith(dir); }) )
                id = fIt.filePath().remove(QRegularExpression("^.*applications/")).replace("/","-");
            else
                id = fIt.fileName();

            // Skip duplicate ids
            if ( std::find_if(desktopEntries.begin(), desktopEntries.end(),
                              [&id](const shared_ptr<StandardIndexItem> & desktopEntry){
                                  return id == desktopEntry->id();
                              }) != desktopEntries.end())
                continue;

            /*
             * Get the data from the desktop file
             */

            // Read the file into a map
            {
            QFile file(fIt.filePath());
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
            icon = XDG::IconLookup::iconPath({xdgStringEscape(getLocalizedKey("Icon", entryMap, loc)),
                                              "application-x-executable",
                                              "exec"});
            if (icon.isNull())
                icon = ":application-x-executable";

            // Check if this is a terminal app
            term = (entryIterator = entryMap.find("Terminal")) != entryMap.end()
                    && entryIterator->second=="true";

            // Try to get the localized genericName
            genericName = xdgStringEscape(getLocalizedKey("GenericName", entryMap, loc));

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

            // Unquote arguments and expand field codes
            QStringList commandline = expandedFieldCodes(Core::ShUtil::split(exec),
                                                         icon, name, fIt.filePath());
            // Malformed exec line. Constraint (1)
            if (commandline.isEmpty())
                continue;

            // Finally we got everything, build the item
            shared_ptr<StandardIndexItem> item = std::make_shared<StandardIndexItem>();
            item->setIconPath(icon);
            item->setId(id);
            item->setText(name);
            item->setCompletion(name);

            // Set subtext/tootip
            if (comment.isEmpty())
                if (genericName.isEmpty())
                    item->setSubtext(exec);
                else
                    item->setSubtext(genericName);
            else
                item->setSubtext(comment);


            // Set keywords
            vector<IndexableItem::IndexString> indexStrings;
            indexStrings.emplace_back(name, UINT_MAX);

            if ( !exec.startsWith("java ")
                 && !exec.startsWith("ruby ")
                 && !exec.startsWith("python ")
                 && !exec.startsWith("perl ")
                 && !exec.startsWith("bash ")
                 && !exec.startsWith("sh ")
                 && !exec.startsWith("dbus-send ")
                 && !exec.startsWith("/") )
                indexStrings.emplace_back(commandline[0], UINT_MAX);  // safe since (1)

            if (useKeywords)
                for (auto & kw : keywords)
                    indexStrings.emplace_back(kw, UINT_MAX/2);

            if (!genericName.isEmpty())
                indexStrings.emplace_back(genericName, UINT_MAX/2   );

            item->setIndexKeywords(std::move(indexStrings));



            /*
             * Build actions
             */

            // Default and root action
            if (term) {
                item->addAction(make_shared<TermAction>("Run", commandline, workingDir, false));
                item->addAction(make_shared<TermAction>("Run as root", QStringList("sudo")+commandline, workingDir, false));
            } else {
                item->addAction(make_shared<ProcAction>("Run", commandline, workingDir));
                item->addAction(make_shared<ProcAction>("Run as root", QStringList("gksudo")+commandline, workingDir));
            }

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
                QStringList commandline = expandedFieldCodes(Core::ShUtil::split(entryIterator->second),
                                                             icon, name, fIt.filePath());
                if (term)
                    item->addAction(make_shared<TermAction>(actionName, commandline, workingDir, false));
                else
                    item->addAction(make_shared<ProcAction>(actionName, commandline, workingDir));

            }


            /*
             * Add item
             */

            desktopEntries.push_back(std::move(item));
        }
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

    d->graphicalSudoPath = QStandardPaths::findExecutable("gksudo");

    // Load settings
    d->offlineIndex.setFuzzy(settings().value(CFG_FUZZY, DEF_FUZZY).toBool());
    d->ignoreShowInKeys = settings().value(CFG_IGNORESHOWINKEYS, DEF_IGNORESHOWINKEYS).toBool();
    d->useKeywords = settings().value(CFG_USEKEYWORDS, DEF_USEKEYWORDS).toBool();

    // If the filesystem changed, trigger the scan
    connect(&d->watcher, &QFileSystemWatcher::directoryChanged,
            std::bind(&Private::startIndexing, d.get()));

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

        // Fuzzy
        d->widget->ui.checkBox_fuzzy->setChecked(d->offlineIndex.fuzzy());
        connect(d->widget->ui.checkBox_fuzzy, &QCheckBox::toggled,
                 this, &Extension::setFuzzy);

        // Use keywords
        d->widget->ui.checkBox_useKeywords->setChecked(d->useKeywords);
        connect(d->widget->ui.checkBox_useKeywords, &QCheckBox::toggled,
                this, [this](bool checked){
            settings().setValue(CFG_USEKEYWORDS, checked);
            d->useKeywords = checked ;
            d->startIndexing();
        });

        // Ignore onlyshowin notshowin keys
        d->widget->ui.checkBox_ignoreShowInKeys->setChecked(d->ignoreShowInKeys);
        connect(d->widget->ui.checkBox_ignoreShowInKeys, &QCheckBox::toggled,
                this, [this](bool checked){
            settings().setValue(CFG_IGNORESHOWINKEYS, checked);
            d->ignoreShowInKeys = checked ;
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
        results.emplace_back(std::static_pointer_cast<Core::StandardIndexItem>(item), 1);

    query->addMatches(std::make_move_iterator(results.begin()),
                      std::make_move_iterator(results.end()));
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
