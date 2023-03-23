// Copyright (c) 2022-2023 Manuel Schneider

#include <QStandardPaths>
#include <QRegularExpression>
#include "plugin.h"
#include "ui_configwidget.h"
ALBERT_LOGGING
using namespace std;
using albert::StandardItem;
using albert::IndexItem;
const char* CFG_IGNORESHOWINKEYS     = "ignore_show_in_keys";
const bool  DEF_IGNORESHOWINKEYS     = false;
const char* CFG_USEKEYWORDS          = "use_keywords";
const bool  DEF_USEKEYWORDS          = false;
const char* CFG_USEGENERICNAME       = "use_generic_name";
const bool  DEF_USEGENERICNAME       = false;
const char* CFG_USEEXEC              = "use_exec";
const bool  DEF_USEEXEC              = false;
const char* CFG_USENONLOCALIZEDNAME  = "use_non_localized_name";
const bool  DEF_USENONLOCALIZEDNAME  = false;

static QString fieldCodesExpanded(const QString & exec, const QString & name, const QString & icon, const QString & de_path)
{
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

static QString xdgStringEscape(const QString & unescaped)
{
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

static QString getLocalizedKey(const QString &key, const map<QString,QString> &entries, const QLocale &loc)
{
    map<QString,QString>::const_iterator it;
    if ( (it = entries.find(QString("%1[%2]").arg(key, loc.name()))) != entries.end()
         || (it = entries.find(QString("%1[%2]").arg(key, loc.name().left(2)))) != entries.end()
         || (it = entries.find(key)) != entries.end())
        return it->second;
    return QString();
}

vector<IndexItem> Plugin::indexApps(const bool &abort) const
{
    vector<IndexItem> results;

    // Get a new index [O(n)]
    QStringList xdg_current_desktop = QString(getenv("XDG_CURRENT_DESKTOP")).split(':', Qt::SkipEmptyParts);
    QLocale loc;

    /*
     * Create a list of desktop files to index (unique ids)
     * To determine the ID of a desktop file, make its full path relative to
     * the $XDG_DATA_DIRS component in which the desktop file is installed,
     * remove the "applications/" prefix, and turn '/' into '-'.
     */

    map<QString /*desktop file id*/, QString /*path*/> desktopFiles;
    for ( const QString &dir : QStandardPaths::standardLocations(QStandardPaths::ApplicationsLocation) ) {
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

        if (abort) return results;

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
                for (const QString &str : entryIterator->second.split(';', Qt::SkipEmptyParts))
                    if (xdg_current_desktop.contains(str))
                        continue;

            // Skip if the current desktop environment is not specified in "OnlyShowIn"
            if ((entryIterator = entryMap.find("OnlyShowIn")) != entryMap.end()) {
                bool found = false;
                for (const QString &str : entryIterator->second.split(';', Qt::SkipEmptyParts))
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
        keywords = xdgStringEscape(getLocalizedKey("Keywords", entryMap, loc)).split(';', Qt::SkipEmptyParts);

        // Try to get the workindir
        if ((entryIterator = entryMap.find("Path")) != entryMap.end())
            workingDir = xdgStringEscape(entryIterator->second);

        // Try to get the keywords
        if ((entryIterator = entryMap.find("Actions")) != entryMap.end())
            actionIdentifiers = xdgStringEscape(entryIterator->second).split(';', Qt::SkipEmptyParts);

//            // Try to get the mimetypes
//            if ((valueIterator = entryMap.find("MimeType")) != entryMap.end())
//                keywords = xdgStringEscape(valueIterator->second).split(';',QString::SkipEmptyParts);


        /*
         * Build the item
         */

        // Field code expandesd commandline
        QString commandLine = fieldCodesExpanded(exec, name, icon, path);

        // Icon path
        QStringList icon_urls = {"xdg:" + icon, "xdg:application-x-executable", "xdg:exec",":application-x-executable"};

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

        // Set index strings
        QStringList index_strings;
        index_strings << name;

        if (useKeywords)
            for (auto & kw : keywords)
                index_strings << kw;

        if (useGenericName && !genericName.isEmpty())
            index_strings << genericName;

        if (useNonLocalizedName && !nonLocalizedName.isEmpty())
            index_strings << nonLocalizedName;

        if (useExec){
            static QStringList excludes = {
                "/",
                "bash ",
                "dbus-send ",
                "env ",
                "java ",
                "perl ",
                "python ",
                "ruby ",
                "sh "
            };
            if (none_of(excludes.begin(), excludes.end(), [&exec](const QString &str){ return exec.startsWith(str); }))
                index_strings << exec.section(QChar(QChar::Space), 0, 0, QString::SectionSkipEmpty);
        }

        /*
         * Build actions
         */

        albert::Actions actionList;

        if (term)
            actionList.emplace_back("run", "Run", [=](){ albert::runTerminal(commandLine, workingDir, true); });
        else
            actionList.emplace_back("run", "Run", [=](){
                albert::runDetachedProcess(QStringList() << "sh" << "-c" << commandLine, workingDir);
            });

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
                actionList.emplace_back(actionName, actionName, [=](){
                    albert::runTerminal(commandLine, workingDir, true);
                });
            else
                actionList.emplace_back(actionName, actionName, [=](){
                    albert::runDetachedProcess(QStringList() << "sh" << "-c" << commandLine, workingDir);
                });
        }

        actionList.emplace_back("reveal-entry", "Open desktop entry in file browser", [=](){
            albert::openUrl(QFileInfo(id_path_pair.second).path());
        });

        auto item = StandardItem::make(id, name, subtext, name, icon_urls, actionList);
        for (const auto &index_string : index_strings){
            results.emplace_back(item, index_string);
        }
    }
    return results;
}

Plugin::Plugin()
{
    qunsetenv("DESKTOP_AUTOSTART_ID");

    // Load settings
    ignoreShowInKeys = settings()->value(CFG_IGNORESHOWINKEYS, DEF_IGNORESHOWINKEYS).toBool();
    useGenericName = settings()->value(CFG_USEGENERICNAME, DEF_USEGENERICNAME).toBool();
    useNonLocalizedName = settings()->value(CFG_USENONLOCALIZEDNAME, DEF_USENONLOCALIZEDNAME).toBool();
    useKeywords = settings()->value(CFG_USEKEYWORDS, DEF_USEKEYWORDS).toBool();
    useExec = settings()->value(CFG_USEEXEC, DEF_USEEXEC).toBool();

    // Paths set on initial update finish
    connect(&fs_watcher_, &QFileSystemWatcher::directoryChanged, this, [this](){ indexer.run(); });

    indexer.parallel = [this](const bool &abort){ return indexApps(abort); };
    indexer.finish = [this](vector<IndexItem> &&result){
        apps = ::move(result);
        updateIndexItems();

        // Finally update the watches (maybe folders changed)
        if (!fs_watcher_.directories().isEmpty())
            fs_watcher_.removePaths(fs_watcher_.directories());

        for (const QString &path : QStandardPaths::standardLocations(QStandardPaths::ApplicationsLocation)) {
            if (QFile::exists(path)) {
                fs_watcher_.addPath(path);
                QDirIterator dit(path, QDir::Dirs|QDir::NoDotAndDotDot, QDirIterator::Subdirectories);
                while (dit.hasNext())
                    fs_watcher_.addPath(dit.next());
            }
        }
    };
    indexer.run();
}

void Plugin::updateIndexItems()
{
    vector<IndexItem> index_items;
    for (const auto & ii : apps)
        index_items.emplace_back(ii.item, ii.string);
    setIndexItems(::move(index_items));
}

QWidget *Plugin::buildConfigWidget()
{
    auto widget = new QWidget;
    Ui::ConfigWidget ui;
    ui.setupUi(widget);

    // Show the app dirs in the label
    ui.label->setText(ui.label->text().replace("__XDG_DATA_DIRS__",
                                               QStandardPaths::standardLocations(QStandardPaths::ApplicationsLocation).join(", ")));

    // Use keywords
    ui.checkBox_useKeywords->setChecked(useKeywords);
    connect(ui.checkBox_useKeywords, &QCheckBox::toggled,
            this, [this](bool checked){
                settings()->setValue(CFG_USEKEYWORDS, checked);
                useKeywords = checked;
                indexer.run();
            });

    // Use generic name
    ui.checkBox_useGenericName->setChecked(useGenericName);
    connect(ui.checkBox_useGenericName, &QCheckBox::toggled,
            this, [this](bool checked){
                settings()->setValue(CFG_USEGENERICNAME, checked);
                useGenericName = checked;
                indexer.run();
            });

    // Use non-localized name
    ui.checkBox_useNonLocalizedName->setChecked(useNonLocalizedName);
    connect(ui.checkBox_useNonLocalizedName, &QCheckBox::toggled,
            this, [this](bool checked){
                settings()->setValue(CFG_USENONLOCALIZEDNAME, checked);
                useNonLocalizedName = checked;
                indexer.run();
            });

    // Use exec
    ui.checkBox_useExec->setChecked(useExec);
    connect(ui.checkBox_useExec, &QCheckBox::toggled,
            this, [this](bool checked){
                settings()->setValue(CFG_USEEXEC, checked);
                useExec = checked;
                indexer.run();
            });

    // Ignore onlyshowin notshowin keys
    ui.checkBox_ignoreShowInKeys->setChecked(ignoreShowInKeys);
    connect(ui.checkBox_ignoreShowInKeys, &QCheckBox::toggled,
            this, [this](bool checked){
                settings()->setValue(CFG_IGNORESHOWINKEYS, checked);
                ignoreShowInKeys = checked;
                indexer.run();
            });

    return widget;
}
