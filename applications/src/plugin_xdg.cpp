// Copyright (c) 2022-2024 Manuel Schneider

/*
 *  https://specifications.freedesktop.org/desktop-entry-spec/1.5/
 *  https://wiki.ubuntu.com/UbuntuDevelopment/Internationalisation/Packaging#Desktop_Entries
 */

#include "plugin.h"
#include "ui_configwidget_xdg.h"
#include <QFileSystemWatcher>
#include <QRegularExpression>
#include <QStandardPaths>
#include <QStringList>
#include <QWidget>
#include <albert/backgroundexecutor.h>
#include <albert/standarditem.h>
#include <albert/util.h>
#include <libintl.h>
#include <locale.h>
#include <memory>
ALBERT_LOGGING_CATEGORY("apps")
using namespace std;
using namespace albert;

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

/// Get escaped value for key
/// \param key The key to fetch the value for
/// \param key The raw desktop entry key/value pairs
/// \return The value for key
static QString stringAt(const map<QString, QString> &entries, const QString &key)
{
    const auto &unescaped = entries.at(key);

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

static QString localeStringAt(const map<QString, QString> &entries, const QString &key, const QLocale &locale)
{
    // Localized values for keys - https://specifications.freedesktop.org/desktop-entry-spec/1.5/ar01s05.html

    // > If a postfixed key occurs, the same key must be also present without the postfix.
    // -> Missing unlocalized string implies the lack of localized strings
    QString unlocalized = stringAt(entries, key);

    // TODO: Properly fetch the localestring (lang_COUNTRY@MODIFIER, lang_COUNTRY, lang@MODIFIER, lang, default value)
    try {
        return stringAt(entries, QString("%1[%2]").arg(key, locale.name()));
    } catch (const out_of_range &) { }

    try {
        return stringAt(entries, QString("%1[%2]").arg(key, locale.name().left(2)));
    } catch (const out_of_range &) { }

    try {
        auto domain = entries.at("X-Ubuntu-Gettext-Domain").toStdString();
        auto msgid = unlocalized.toStdString();
        // The resulting string is statically allocated and must not be modified or freed
        // Returns msgid on lookup failure
        // https://linux.die.net/man/3/dgettext
        return QString::fromUtf8(dgettext(domain.c_str(), msgid.c_str()));
    } catch (const out_of_range &) { }

    return unlocalized;
}

static map<QString, map<QString,QString>> readDesktopEntry(const QString &path)
{
    map<QString, map<QString,QString>> result;
    if (QFile file(path); file.open(QIODevice::ReadOnly| QIODevice::Text)) {
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
            result[currentGroup].emplace(line.section('=', 0,0).trimmed(),
                                         line.section('=', 1, -1).trimmed());
        }
        file.close();
    }
    else
        throw runtime_error(QString("Failed opening file '%1': %2").arg(path, file.errorString()).toStdString());

    return result;
}

static std::pair<shared_ptr<StandardItem>, QStringList> parseDesktopEntry(Plugin &p, const QString &id, const QString &path)
{
    const map<QString,map<QString,QString>> desktopEntry = readDesktopEntry(path);

    try {

        // Recognized desktop entry keys - https://specifications.freedesktop.org/desktop-entry-spec/1.5/ar01s06.html
        // Possible value types - https://specifications.freedesktop.org/desktop-entry-spec/1.5/ar01s04.html


        // Parse item values

        auto general = desktopEntry.at(QStringLiteral("Desktop Entry"));
        QLocale locale;

        // Type - string, REQUIRED
        try {
            if (stringAt(general, QStringLiteral("Type")) != QStringLiteral("Application"))
                throw runtime_error("Desktop entries of type other than 'Application' are not handled yet.");
        } catch (const out_of_range &) {
            throw runtime_error("Desktop entry does not contain a 'Desktop Entry' section.");
        }

        // NoDisplay - boolean
        try {
            if (general.at(QStringLiteral("NoDisplay")) == QStringLiteral("true"))
                throw runtime_error("Key 'NoDisplay' set.");
        } catch (const out_of_range &) { }

        // OnlyShowIn, NotShowIn - string(s)
        if (!p.ignore_show_in_keys()) {
            const QStringList xdg_current_desktop(QString(getenv("XDG_CURRENT_DESKTOP")).split(':', Qt::SkipEmptyParts));
            try {
                for (const auto &desktop_environment : stringAt(general, QStringLiteral("NotShowIn")).split(';', Qt::SkipEmptyParts))
                    if (xdg_current_desktop.contains(desktop_environment))
                        throw runtime_error("Desktop entry excluded by 'NotShowIn'.");
            } catch (const out_of_range &) { }

            try {
                if (!ranges::any_of(stringAt(general, QStringLiteral("OnlyShowIn")).split(';', Qt::SkipEmptyParts),
                                    [&](const auto &desktop_env){ return xdg_current_desktop.contains(desktop_env); }))
                    throw runtime_error("Desktop entry excluded by 'OnlyShowIn'.");
            } catch (const out_of_range &) { }
        }

        // Name - localestring, REQUIRED
        QString name;
        try {
            name = localeStringAt(general, QStringLiteral("Name"), locale);
        } catch (const out_of_range &) {
            throw runtime_error("Desktop entry does not contain 'Name' key.");
        }

        QString nonLocalizedName;
        try {
            nonLocalizedName = stringAt(general, QStringLiteral("Name"));
        } catch (const out_of_range &) { }

        // GenericName - localestring
        QString genericName;
        try {
            genericName = localeStringAt(general, QStringLiteral("GenericName"), locale);
        } catch (const out_of_range &) { }

        // Comment - localestring
        QString comment;
        try {
            comment = localeStringAt(general, QStringLiteral("Comment"), locale);
        } catch (const out_of_range &) { }

        // Keywords - localestring(s)
        QStringList keywords;
        try {
            keywords = localeStringAt(general, QStringLiteral("Keywords"), locale).split(';', Qt::SkipEmptyParts);
        } catch (const out_of_range &) { }

        // Icon - iconstring (xdg icon naming spec)
        QString icon;
        try {
            icon = localeStringAt(general, QStringLiteral("Icon"), locale);
        } catch (const out_of_range &) { }

        // Exec - string
        QString exec;
        try {
            exec = fieldCodesExpanded(stringAt(general, QStringLiteral("Exec")), name, icon, path);
        } catch (const out_of_range &) {
            throw runtime_error("Desktop entry does not contain 'Exec' key.");
        }

        // Path - string
        QString working_dir;
        try {
            working_dir = stringAt(general, QStringLiteral("Path"));
        } catch (const out_of_range &) { }

        // Terminal - boolean
        bool term = false;
        try {
            term = general.at(QStringLiteral("Terminal")) == QStringLiteral("true");
        } catch (const out_of_range &) { }

        // Actions - string(s)
        QStringList actions;
        try {
            actions = stringAt(general, QStringLiteral("Actions")).split(';', Qt::SkipEmptyParts);
        } catch (const out_of_range &) { }


        // --


        vector<Action> actionList;

        std::function<void()> fun;
        if (term)
            fun = [=]() -> void { albert::runTerminal(exec, working_dir, true); };
        else
            fun = [=]() -> void { albert::runDetachedProcess(QStringList() << "sh" << "-c" << exec, working_dir); };
        actionList.emplace_back("launch", Plugin::tr("Launch app"), fun );

        for (const QString &action_identifier : actions)
        {
            try
            {
                auto action_section = desktopEntry.at(QString("Desktop Action %1").arg(action_identifier));

                // Name - localestring, REQUIRED
                QString action_name;
                try {
                    action_name = localeStringAt(action_section, QStringLiteral("Name"), locale);
                } catch (const out_of_range &) {
                    throw runtime_error("Desktop action does not contain 'Name' key.");
                }

                // Icon - iconstring (xdg icon naming spec)
                QString action_icon;
                try {
                    action_icon = localeStringAt(action_section, QStringLiteral("Icon"), locale);
                } catch (const out_of_range &) { }

                // Exec - string
                QString action_exec;
                try {
                    action_exec = fieldCodesExpanded(stringAt(action_section, QStringLiteral("Exec")),
                                                     name, icon, path);
                } catch (const out_of_range &) {
                    throw runtime_error("Desktop action does not contain 'Exec' key.");
                }

                std::function<void()> action_fun;
                if (term)
                    action_fun = [=](){ albert::runTerminal(action_exec, working_dir, true); };
                else
                    action_fun =  [=](){ albert::runDetachedProcess(QStringList() << "sh" << "-c" << action_exec, working_dir); };
                actionList.emplace_back(action_name, action_name, action_fun);
            }
            catch (const out_of_range &)
            {
                WARN << "Desktop action" << action_identifier << "missing:" << path;
            }
        }

        actionList.emplace_back("reveal-entry", "Open desktop entry in file browser", [=](){
            albert::openUrl(QFileInfo(path).path());
        });


        // --


        QStringList index_strings;
        index_strings << name;
        if (p.use_keywords())
            index_strings << keywords;
        if (p.use_generic_name() && !genericName.isEmpty())
            index_strings << genericName;
        if (p.use_non_localized_name() && !nonLocalizedName.isEmpty())
            index_strings << nonLocalizedName;
        if (p.use_exec()){
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
            if (ranges::none_of(excludes, [&exec](const QString &str){ return exec.startsWith(str); }))
                index_strings << exec.section(QChar(QChar::Space), 0, 0, QString::SectionSkipEmpty);
        }
        index_strings.removeDuplicates();

        QString subtext;
        if (comment.isEmpty())
            subtext = keywords.join(", ");
        else
            subtext = comment;

        QStringList icon_urls = {"xdg:" + icon, QStringLiteral(":unkown")};

        return {StandardItem::make(id, name, subtext, name, icon_urls, actionList), index_strings};

    } catch (const out_of_range &) {
        throw runtime_error("Desktop entry does not contain a 'Desktop Entry' section.");
    }
}


class Plugin::Private
{
public:
    BackgroundExecutor<vector<IndexItem>> indexer;
    QFileSystemWatcher fs_watcher;
};


Plugin::Plugin() : d(make_unique<Private>())
{
    qunsetenv("DESKTOP_AUTOSTART_ID");

    // Load settings
    auto s = settings();
    restore_ignore_show_in_keys(s);
    restore_use_exec(s);
    restore_use_generic_name(s);
    restore_use_keywords(s);
    restore_use_non_localized_name(s);

    for (auto f : {&Plugin::ignore_show_in_keys_changed,
                   &Plugin::use_exec_changed,
                   &Plugin::use_generic_name_changed,
                   &Plugin::use_keywords_changed,
                   &Plugin::use_non_localized_name_changed})
        connect(this, f, this, &Plugin::updateIndexItems);

    // Paths set on initial update finish
    connect(&d->fs_watcher, &QFileSystemWatcher::directoryChanged,
            this, [this](){ d->indexer.run(); });

    d->indexer.parallel = [this](const bool &abort)
    {
        // Get a map of unique desktop entries according to the spec
        map<QString /*desktop file id*/, QString /*path*/> desktopFiles;
        for (const QString &dir : QStandardPaths::standardLocations(QStandardPaths::ApplicationsLocation))
        {
            QDirIterator fIt(dir, QStringList("*.desktop"), QDir::Files,
                             QDirIterator::Subdirectories|QDirIterator::FollowSymlinks);

            while (!fIt.next().isEmpty())
            {
                // To determine the ID of a desktop file, make its full path relative to
                // the $XDG_DATA_DIRS component in which the desktop file is installed,
                // remove the "applications/" prefix, and turn '/' into '-'.
                static QRegularExpression re("^.*applications/");
                QString desktopFileId = fIt.filePath().remove(re).replace("/","-");

                if (const auto &[it, success] = desktopFiles.emplace(desktopFileId, fIt.filePath()); !success)
                    DEBG << QString("Desktop file '%1' will be skipped: Shadowed by '%2'").arg(fIt.filePath(), desktopFiles[desktopFileId]);
            }
        }

        // Index the unique desktop files
        vector<IndexItem> results;
        for (const auto &[id, path] : desktopFiles)
        {
            if (abort)
                return results;

            try {
                const auto &[item, index_strings] = parseDesktopEntry(*this, id, path);
                for (auto &index_string : index_strings)
                    results.emplace_back(item, index_string);
            } catch (const std::exception &e) {
                DEBG << QString("Skipped desktop entry '%1': %2").arg(path, e.what());
            }

        }
        return results;
    };

    d->indexer.finish = [this](vector<IndexItem> &&result)
    {
        INFO << QStringLiteral("Indexed %1 apps [%2 ms]")
                    .arg(result.size()).arg(d->indexer.runtime.count());

        vector<IndexItem> index_items;
        for (const auto & ii : result)
            index_items.emplace_back(ii.item, ii.string);
        setIndexItems(::move(index_items));

        // Finally update the watches (maybe folders changed)
        if (!d->fs_watcher.directories().isEmpty())
            d->fs_watcher.removePaths(d->fs_watcher.directories());

        for (const QString &path : QStandardPaths::standardLocations(QStandardPaths::ApplicationsLocation)) {
            if (QFile::exists(path)) {
                d->fs_watcher.addPath(path);
                QDirIterator dit(path, QDir::Dirs|QDir::NoDotAndDotDot, QDirIterator::Subdirectories);
                while (dit.hasNext())
                    d->fs_watcher.addPath(dit.next());
            }
        }
    };
}

Plugin::~Plugin()= default;

QString Plugin::defaultTrigger() const
{ return QStringLiteral("apps "); }

void Plugin::updateIndexItems()
{ d->indexer.run(); }

QWidget *Plugin::buildConfigWidget()
{
    auto widget = new QWidget;
    Ui::ConfigWidget ui;
    ui.setupUi(widget);

    // Show the app dirs in the label
    ui.label->setText(ui.label->text().replace("__XDG_DATA_DIRS__",
                                               QStandardPaths::standardLocations(QStandardPaths::ApplicationsLocation).join(", ")));

    ALBERT_PROPERTY_CONNECT_CHECKBOX(this, ignore_show_in_keys, ui.checkBox_ignoreShowInKeys);
    ALBERT_PROPERTY_CONNECT_CHECKBOX(this, use_exec, ui.checkBox_useExec);
    ALBERT_PROPERTY_CONNECT_CHECKBOX(this, use_generic_name, ui.checkBox_useGenericName);
    ALBERT_PROPERTY_CONNECT_CHECKBOX(this, use_keywords, ui.checkBox_useKeywords);
    ALBERT_PROPERTY_CONNECT_CHECKBOX(this, use_non_localized_name, ui.checkBox_useNonLocalizedName);

    return widget;
}
