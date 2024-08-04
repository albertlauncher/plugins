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
#include <memory>
#include <format>
#include <QMessageBox>
ALBERT_LOGGING_CATEGORY("apps")
using namespace std;
using namespace albert;

static Plugin* plugin = nullptr;
static const char* CFG_TERM = "terminal";


class DesktopEntryParser
{
    map<QString, map<QString,QString>> data;
    QLocale locale;

    /**
     * Get escaped value for key
     *
     * The escape sequences `\s`, `\n`, `\t`, `\r`, and `\\` are supported for
     * values of type string and localestring, meaning ASCII space, newline,
     * tab, carriage return, and backslash, respectively.
     *
     * http://standards.freedesktop.org/desktop-entry-spec/latest/ar01s03.html
     *
     * \param key The key to fetch the value for
     * \param key The raw desktop entry key/value pairs
     * \return The value for key
     */
    static QString escaped(const QString &unescaped) noexcept
    {
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

public:

    DesktopEntryParser(const QString &path)
    {
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
                data[currentGroup].emplace(line.section('=', 0,0).trimmed(),
                                            line.section('=', 1, -1).trimmed());
            }
            file.close();
        }
        else
            throw runtime_error(QString("Failed opening file '%1': %2").arg(path, file.errorString()).toStdString());
    }

    class SectionDoesNotExist : public out_of_range { using out_of_range::out_of_range; };
    class KeyDoesNotExist : public out_of_range { using out_of_range::out_of_range; };

    /// This is the most basic function since it returns the raw string.
    ///
    /// Values of type string may contain all ASCII characters except for
    /// control characters.
    ///
    /// @returns The raw string value of the key in section
    /// @param section The section to get the value from
    /// @param key The key to the value for
    /// @throws out_of_range if lookup failed
    QString getValue(const QString &section, const QString &key) const
    {
        try {
            auto &s = data.at(section);
            try {
                return escaped(s.at(key));
            } catch (const out_of_range&) {
                throw KeyDoesNotExist(format("Section '{}' does not contain a key '{}'.",
                                             section.toStdString(), key.toStdString()));
            }
        } catch (const out_of_range&) {
            throw SectionDoesNotExist(format("Desktop entry does not contain a section '{}'.",
                                             section.toStdString()));
        }
    }

    /// Values of type string may contain all ASCII characters except for
    /// control characters.
    QString getString(const QString &section, const QString &key) const
    {
        return escaped(getValue(section, key));
    }

    /// Values of type localestring are user displayable, and are encoded in UTF-8.
    /// https://specifications.freedesktop.org/desktop-entry-spec/1.5/ar01s05.html
    // TODO: Properly fetch the localestring
    //       (lang_COUNTRY@MODIFIER, lang_COUNTRY, lang@MODIFIER, lang, default value)
    QString getLocaleString(const QString &section, const QString &key)
    {
        try {
            return getString(section, QString("%1[%2]").arg(key, locale.name()));
        } catch (const out_of_range&) { }

        try {
            return getString(section, QString("%1[%2]").arg(key, locale.name().left(2)));
        } catch (const out_of_range&) { }

        QString unlocalized = getString(section, key);

        try {
            auto domain = getString(section, QStringLiteral("X-Ubuntu-Gettext-Domain"));
            // The resulting string is statically allocated and must not be modified or freed
            // Returns msgid on lookup failure
            // https://linux.die.net/man/3/dgettext
            return QString::fromUtf8(dgettext(domain.toStdString().c_str(),
                                              unlocalized.toStdString().c_str()));
        } catch (const out_of_range&) { }

        return unlocalized;
    }

    /// Values of type iconstring are the names of icons; these may be
    /// absolute paths, or symbolic names for icons located using the
    /// algorithm described in the Icon Theme Specification. Such values
    /// are not user-displayable, and are encoded in UTF-8.
    QString getIconString(const QString &section, const QString &key)
    {
        return getString(section, key);
    }

    /// Values of type boolean must either be the string true or false.
    bool getBoolean(const QString &section, const QString &key)
    {
        auto raw = getString(section, key);  // throws
        if (raw == QStringLiteral("true"))
            return true;
        else if (raw == QStringLiteral("false"))
            return false;
        else
            throw runtime_error(format("Value for key '{}' in section '{}' is neither true nor false.",
                                       key.toStdString(), section.toStdString()));
    }

    /// Values of type numeric must be a valid floating point number as
    /// recognized by the %f specifier for scanf in the C locale.
    double getNumeric(const QString &, const QString &)
    {
        throw runtime_error("Not implemented.");
    }

    /// Split Exec string according to spec
    /// https://specifications.freedesktop.org/desktop-entry-spec/latest/exec-variables.html
    static optional<QStringList> splitExec(const QString &s) noexcept
    {
        QStringList tokens;
        QString token;
        auto c = s.begin();

        while (c != s.end())
        {
            if (*c == QChar::Space)  // separator
            {
                if (!token.isEmpty())
                {
                    tokens << token;
                    token.clear();
                }
            }

            else if (*c == '"')  // quote
            {
                ++c;

                while (c != s.end())
                {
                    if (*c == '"')  // quote termination
                        break;

                    else if (*c == '\\')  // escape
                    {
                        ++c;
                        if(c == s.end())
                        {
                            WARN << QString("Unterminated escape in %1").arg(s);
                            return {};  // unterminated escape
                        }

                        else if (QStringLiteral(R"("`$\)").contains(*c))
                            token.append(*c);

                        else
                        {
                            WARN << QString("Invalid escape '%1' at '%2': %3")
                                        .arg(*c).arg(distance(c, s.begin())).arg(s);
                            return {};  // invalid escape
                        }
                    }

                    else
                        token.append(*c);  // regular char

                    ++c;
                }

                if (c == s.end())
                {
                    WARN << QString("Unterminated escape in %1").arg(s);
                    return {};  // unterminated quote
                }
            }

            else
                token.append(*c);  // regular char

            ++c;

        }

        if (!token.isEmpty())
            tokens << token;

        return tokens;
    }
};

class XDGApp : public albert::Item
{
    QString id_;
    QString name_;
    QString description_;
    QString path_;
    QString icon_;
    QStringList exec_;
    QString working_dir_;
    struct DesktopAction {
        QString id_;
        QString name_;
        QStringList exec_;
    };
    vector<DesktopAction> desktop_actions_;
    bool term_ = false;

    /**
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
    static QStringList fieldCodesExpanded(const QStringList & exec,
                                          const QString & localized_name,
                                          const QString & icon,
                                          const QString & de_path,
                                          QUrl url ={})
    {
        QStringList c;
        for (const auto &t : exec)
        {
            if (t == QStringLiteral("%%"))
                c << QStringLiteral("%");
            else if (t == "%f" || t == "%F")
            {
                if (!url.isEmpty())
                    c << url.toLocalFile();
            }
            else if (t == "%u" || t == "%U")
            {
                if (!url.isEmpty())
                    c << url.toString();
            }
            else if (t == "%i" && !icon.isNull())
                c << "--icon" << icon;
            else if (t == "%c")
                c << localized_name;
            else if (t == "%k")
                c << de_path;
            else if (t == "%v" || t == "%m" || t == "%d" || t == "%D" || t == "%n" || t == "%N")
                ;  // Skipping deprecated field codes
            else
                c << t;
        }
        return c;
    }

    void runCommandline(const QStringList &commandline, const QString &working_dir) const
    {
        if (term_)
            plugin->runTerminal(commandline, working_dir);
        else
            albert::runDetachedProcess(commandline, working_dir);
    }

public:

    struct ParseOptions
    {
        bool ignore_show_in_keys;
        bool use_exec;
        bool use_generic_name;
        bool use_keywords;
        bool use_non_localized_name;
    };

    struct ParsedEntry
    {
        shared_ptr<XDGApp> item;
        QStringList index_strings;
        QStringList mime_types;
    };

    ///
    /// Parse the desktop entry
    /// Produces the item and byproducts used for indexing
    ///
    /// Recognized desktop entry keys - https://specifications.freedesktop.org/desktop-entry-spec/1.5/ar01s06.html
    /// Possible value types - https://specifications.freedesktop.org/desktop-entry-spec/1.5/ar01s04.html
    ///
    static ParsedEntry parseDesktopEntry(const QString &id, const QString &path, ParseOptions po)
    {
        DesktopEntryParser p(path);
        auto root_section = QStringLiteral("Desktop Entry");
        ParsedEntry pe;
        pe.item = make_shared<XDGApp>();
        pe.item->id_ = id;
        pe.item->path_ = path;

        // Type - string, REQUIRED to be Application
        if (p.getString(root_section, QStringLiteral("Type")) != QStringLiteral("Application"))
            throw runtime_error("Desktop entries of type other than 'Application' are not handled yet.");

        // NoDisplay - boolean, must not be true
        try {
            if (p.getBoolean(root_section, QStringLiteral("NoDisplay")))
                throw runtime_error("Desktop entry excluded by 'NoDisplay'.");
        } catch (const out_of_range &) { }

        if (!po.ignore_show_in_keys)
        {
            const auto desktops(QString(getenv("XDG_CURRENT_DESKTOP")).split(':', Qt::SkipEmptyParts));

            // NotShowIn - string(s), if exists must not be in XDG_CURRENT_DESKTOP
            try {
                if (ranges::any_of(p.getString(root_section, QStringLiteral("NotShowIn")).split(';', Qt::SkipEmptyParts),
                                   [&](const auto &de){ return desktops.contains(de); }))
                    throw runtime_error("Desktop entry excluded by 'NotShowIn'.");
            } catch (const out_of_range &) { }

            // OnlyShowIn - string(s), if exists has to be in XDG_CURRENT_DESKTOP
            try {
                if (!ranges::any_of(p.getString(root_section, QStringLiteral("OnlyShowIn")).split(';', Qt::SkipEmptyParts),
                                    [&](const auto &de){ return desktops.contains(de); }))
                    throw runtime_error("Desktop entry excluded by 'OnlyShowIn'.");
            } catch (const out_of_range &) { }
        }

        // Non localized name - string, REQUIRED
        auto name = p.getString(root_section, QStringLiteral("Name"));
        if (po.use_non_localized_name)
            pe.index_strings << name; // always call the above, make sure it throws

        // Localized name - localestring, may equal name if no localizations available
        // No need to catch despite optional, since falls back to name, which is required
        auto localized_name = p.getLocaleString(root_section, QStringLiteral("Name"));
        pe.item->name_ = localized_name;
        pe.index_strings << localized_name;

        // Exec - string, REQUIRED despite not strictly by standard
        try
        {
            pe.item->exec_ = DesktopEntryParser::splitExec(p.getString(root_section, QStringLiteral("Exec"))).value();
            if (pe.item->exec_.isEmpty())
                throw runtime_error("Empty Exec value.");
        }
        catch (const bad_optional_access&)
        {
            throw runtime_error("Malformed Exec value.");
        }

        if (po.use_exec)
        {
            static QStringList excludes = {
                "/",
                "bash ",
                "dbus-send ",
                "env ",
                "flatpak ",
                "java ",
                "perl ",
                "python ",
                "ruby ",
                "sh "
            };

            if (ranges::none_of(excludes, [&](const QString &str){ return pe.item->exec_.startsWith(str); }))
                pe.index_strings << pe.item->exec_.at(0);
        }

        // Comment - localestring
        try {
            pe.item->description_ = p.getLocaleString(root_section, QStringLiteral("Comment"));
        } catch (const out_of_range &) { }

        // Keywords - localestring(s)
        try {
            auto keywords = p.getLocaleString(root_section, QStringLiteral("Keywords")).split(';', Qt::SkipEmptyParts);
            if (pe.item->description_.isEmpty())
                pe.item->description_ = keywords.join(", ");
            if (po.use_keywords)
                pe.index_strings << keywords;
        } catch (const out_of_range &) { }

        // Icon - iconstring (xdg icon naming spec)
        try {
            pe.item->icon_ = p.getLocaleString(root_section, QStringLiteral("Icon"));
        } catch (const out_of_range &) { }

        // Path - string
        try {
            pe.item->working_dir_ = p.getString(root_section, QStringLiteral("Path"));
        } catch (const out_of_range &) { }

        // Terminal - boolean
        try {
            pe.item->term_ = p.getBoolean(root_section, QStringLiteral("Terminal"));
        } catch (const out_of_range &) { }

        // GenericName - localestring
        if (po.use_generic_name)
            try { pe.index_strings << p.getLocaleString(root_section, QStringLiteral("GenericName")); }
            catch (const out_of_range &) { }

        // Actions - string(s)
        try {
            auto action_ids = p.getString(root_section, QStringLiteral("Actions")).split(';', Qt::SkipEmptyParts);
            for (const QString &action_id : action_ids)
            {
                try
                {
                    const auto action_section = QString("Desktop Action %1").arg(action_id);

                    // Name - localestring, REQUIRED
                    auto _name = p.getLocaleString(action_section, QStringLiteral("Name"));

                    // Exec - string, REQUIRED despite not strictly by standard
                    auto _exec = DesktopEntryParser::splitExec(p.getString(action_section, QStringLiteral("Exec")));
                    if (!_exec)
                        throw runtime_error("Malformed Exec value.");
                    else if (_exec.value().isEmpty())
                        throw runtime_error("Empty Exec value.");
                    else

                    pe.item->desktop_actions_.emplace_back(action_id, _name, _exec.value());
                }
                catch (const out_of_range &e)
                {
                    WARN << "Desktop action" << action_id << "skipped: " << e.what();
                }
            }
        } catch (const out_of_range &) { }

        // MimeType, string(s)
        try {
            pe.mime_types = p.getString(root_section, QStringLiteral("MimeType")).split(';', Qt::SkipEmptyParts);
        } catch (const out_of_range &) { }

        pe.index_strings.removeDuplicates();
        pe.mime_types.removeDuplicates();

        return pe;
    }

    void run(const QUrl &url = {}, const QStringList additional_args = {}, const QString working_dir = {}) const
    {
        auto exec = QStringList(exec_) << additional_args;
        auto wd = working_dir.isEmpty() ? working_dir_ : working_dir;
        runCommandline(fieldCodesExpanded(exec, name_, icon_, wd, url), wd);
    }

    QString name() const { return name_; }

    // albert::Item interface
    QString id() const override { return id_; }
    QString text() const override { return name_; }
    QString subtext() const override { return description_; }
    QString inputActionText() const override { return name_; }
    QStringList iconUrls() const override
    {
        if (QFileInfo(icon_).isAbsolute())
            return { icon_ };
        else
            return { QString("xdg:%1").arg(icon_) };
    }
    vector<Action> actions() const override
    {
        vector<Action> actions;

        actions.emplace_back("launch", Plugin::tr("Launch app"), [this]{ run(); });

        for (const auto &da : desktop_actions_)
            actions.emplace_back(
                QString("action-%1").arg(da.id_), da.name_,
                [this, &da]{ runCommandline(fieldCodesExpanded(da.exec_, name_, icon_, working_dir_), working_dir_); }
                );

        actions.emplace_back("reveal-entry",
                             Plugin::tr("Open desktop entry in file browser"),
                             [this](){ albert::openUrl(path_); });

        return actions;
    }
};


class Plugin::Private
{
public:
    BackgroundExecutor<map<shared_ptr<XDGApp>, QStringList>> indexer;
    vector<shared_ptr<XDGApp>> apps;
    map<QString, shared_ptr<XDGApp>> terminals;
    XDGApp *user_terminal = nullptr;

    // Supported terminals
    // Desktop id > command execution arguments
    const map<QString, QStringList> exec_args
    {
        {"alacritty", {"-e"}},
        {"app.devsuite.ptyxis", {"--"}},  // Flatpak
        {"blackbox", {"--"}},
        {"com.gexperts.tilix", {"-e"}},
        {"com.raggesilver.blackbox", {"--"}},  // Flatpak
        {"console", {"-e"}},
        {"contour", {"execute"}},
        {"cool-retro-term", {"-e"}},
        {"deepin-terminal", {"-e"}},
        {"deepin-terminal-gtk", {"-e"}},
        {"elementary-terminal", {"-x"}},
        {"kitty", {"--"}},
        {"konsole", {"-e"}},
        {"lxterminal", {"-e"}},
        {"mate-terminal", {"-x"}},
        {"org.codeberg.dnkl.foot", {}},
        {"org.contourterminal.contour", {"execute"}},  // Flatpak
        {"org.gnome.console", {"-e"}},
        {"org.gnome.terminal", {"--"}},
        {"org.kde.konsole", {"-e"}},  // Flatpak
        {"org.wezfurlong.wezterm", {"-e"}},  // Flatpak
        {"ptyxis", {"--"}},
        {"qterminal", {"-e"}},
        {"roxterm", {"-x"}},
        {"st", {"-e"}},
        // See #1177 and https://github.com/gnome-terminator/terminator/issues/702 and 660
        // TODO remove in future. Like in 2027 🤷
        // {"terminator (<=2.1.2)", {"-u", "-g", "/dev/null", "-x"}},
        {"terminator", {"-u", "-x"}},
        {"terminology", {"-e"}},
        {"termite", {"-e"}},
        {"tilix", {"-e"}},
        {"urxvt", {"-e"}},
        {"uxterm", {"-e"}},
        {"wezterm", {"-e"}},
        {"xfce-terminal", {"-x"}},
        {"xterm", {"-e"}},
    };
};

Plugin::Plugin() : d(make_unique<Private>())
{
    qunsetenv("DESKTOP_AUTOSTART_ID");
    plugin = this;


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


    // File watches

    for (const auto &path : QStandardPaths::standardLocations(QStandardPaths::ApplicationsLocation))
        for (auto dit = QDirIterator(path, QDir::Dirs|QDir::NoDotDot, QDirIterator::Subdirectories); dit.hasNext();)
            fs_watcher_.addPath(QFileInfo(dit.next()).canonicalFilePath());

    connect(&fs_watcher_, &QFileSystemWatcher::directoryChanged,
            this, [this](){ d->indexer.run(); });


    // Indexer

    d->indexer.parallel = [this](const bool &abort)
    {
        // Get a map of unique desktop entries according to the spec
        map<QString /*desktop file id*/, QString /*path*/> desktopFiles;
        for (const QString &dir : appDirectories())
        {
            QDirIterator fIt(dir, QStringList("*.desktop"), QDir::Files,
                             QDirIterator::Subdirectories|QDirIterator::FollowSymlinks);

            while (!fIt.next().isEmpty())
            {
                // To determine the ID of a desktop file, make its full path relative to
                // the $XDG_DATA_DIRS component in which the desktop file is installed,
                // remove the "applications/" prefix, and turn '/' into '-'.
                static QRegularExpression re("^.*applications/");
                QString desktopFileId = fIt.filePath().remove(re).replace("/","-").chopped(8).toLower();  // sizeof '.desktop'

                // CRIT << desktopFileId;
                // for (auto &[id, launch_options] : d->launch_options)
                //     if (desktopFileId == id)
                //         INFO << id;

                if (const auto &[it, success] = desktopFiles.emplace(desktopFileId, fIt.filePath()); !success)
                    DEBG << QString("Desktop file '%1' will be skipped: Shadowed by '%2'").arg(fIt.filePath(), desktopFiles[desktopFileId]);
            }
        }

        // Index the unique desktop files
        map<shared_ptr<XDGApp>, QStringList> results;
        for (const auto &[id, path] : desktopFiles)
        {
            if (abort)
                return results;

            try
            {
                XDGApp::ParseOptions po{
                    .ignore_show_in_keys = ignore_show_in_keys(),
                    .use_exec = use_exec(),
                    .use_generic_name = use_generic_name(),
                    .use_keywords = use_keywords(),
                    .use_non_localized_name = use_non_localized_name()
                };

                XDGApp::ParsedEntry pe = XDGApp::parseDesktopEntry(id, path, po);

                results.emplace(pe.item, pe.index_strings);
            }
            catch (const exception &e)
            {
                DEBG << QString("Skipped desktop entry '%1': %2").arg(path, e.what());
            }

        }
        return results;
    };

    d->indexer.finish = [this](map<shared_ptr<XDGApp>, QStringList> &&apps)
    {
        INFO << QStringLiteral("Indexed %1 apps [%2 ms]")
                    .arg(apps.size()).arg(d->indexer.runtime.count());

        // Populate apps and index
        d->apps.clear();
        vector<IndexItem> index_items;
        for (const auto &[item, strings] : apps)
        {
            d->apps.emplace_back(item);
            for (const auto & string : strings)
                index_items.emplace_back(item, string);
        }
        setIndexItems(::move(index_items));

        // Populate terminals
        // Filter supported terms by availability using destkop id
        d->terminals.clear();
        for (auto &app: d->apps)
            for (auto &[id, launch_options] : d->exec_args)
                if (app->id() == id)
                    d->terminals.emplace(id, app);

        // Set user_terminal depending on config
        if (d->terminals.empty())
        {
            WARN << "No terminals available.";
            d->user_terminal = nullptr;
        }
        else if (auto s = settings(); !s->contains(CFG_TERM))  // unconfigured
        {
            d->user_terminal = d->terminals.begin()->second.get();  // guaranteed to exist since not empty
            WARN << QString("No terminal configured. Using %1.").arg(d->user_terminal->name());
        }
        else  // user configured
        {
            auto config_term = s->value(CFG_TERM).toString();
            try {
                d->user_terminal = d->terminals.at(config_term).get();
            } catch (const out_of_range &e) {
                d->user_terminal = d->terminals.begin()->second.get();  // guaranteed to exist since not empty
                WARN << QString("Configured terminal '%1'  does not exist. Using %2.")
                            .arg(config_term, d->user_terminal->name());
            }
        }

        // runTerminal();
    };
}

Plugin::~Plugin() = default;

void Plugin::updateIndexItems()  { d->indexer.run(); }

void Plugin::runTerminal(const QString &script, const QString &working_dir, bool close_on_exit) const
{
    auto shell = userShell();
    auto args = QStringList() << shell;

    if (!script.isEmpty())
    {
        args << "-i" << "-c";

        if (close_on_exit)
            args << script;
        else
            args << QString("%1 ; exec %2").arg(script, shell);
    }

    runTerminal(args, working_dir);
}

void Plugin::runTerminal(const QStringList &commandline, const QString &working_dir) const
{
    if (!d->user_terminal)
    {
        QMessageBox::warning(nullptr, {},
                             tr("Failed to run terminal. No supported terminal available."));
        return;
    }

    // at() should be safe here because user_terminal is only set if the id exists in exec_args
    QStringList args;
    args << d->exec_args.at(d->user_terminal->id()) << commandline;
    d->user_terminal->run({}, args, working_dir);
}

vector<Action> Plugin::actions(const QUrl&) const
{
    qFatal("NI");
    return {};
}

QWidget *Plugin::buildConfigWidget()
{
    auto widget = new QWidget;
    Ui::ConfigWidget ui;
    ui.setupUi(widget);

    ALBERT_PROPERTY_CONNECT_CHECKBOX(this, ignore_show_in_keys, ui.checkBox_ignoreShowInKeys);
    ALBERT_PROPERTY_CONNECT_CHECKBOX(this, use_exec, ui.checkBox_useExec);
    ALBERT_PROPERTY_CONNECT_CHECKBOX(this, use_generic_name, ui.checkBox_useGenericName);
    ALBERT_PROPERTY_CONNECT_CHECKBOX(this, use_keywords, ui.checkBox_useKeywords);
    ALBERT_PROPERTY_CONNECT_CHECKBOX(this, use_non_localized_name, ui.checkBox_useNonLocalizedName);

    for (const auto &[id, app] : d->terminals)
    {
        ui.comboBox_terminals->addItem(app->name(), id);
        if (id == d->user_terminal->id())  // is current
            ui.comboBox_terminals->setCurrentIndex(ui.comboBox_terminals->count()-1);
    }

    connect(ui.comboBox_terminals, qOverload<int>(&QComboBox::currentIndexChanged),
            this, [this, ui](int index)
            {
                auto id = ui.comboBox_terminals->itemData(index).toString();
                if (auto it = d->terminals.find(id); it != d->terminals.end())
                {
                    d->user_terminal = it->second.get();
                    settings()->setValue(CFG_TERM, it->first);
                }
            });

    QString t = "https://github.com/albertlauncher/albert/issues/new"
                "?assignees=ManuelSchneid3r&title=Terminal+[terminal-name]+missing"
                "&body=Post+an+xterm+-e+compatible+commandline.";
    t = tr(R"(Report missing terminals <a href="%1">here</a>.)").arg(t);
    t = QString(R"(<span style="font-size:9pt; color:#808080;">%1</span>)").arg(t);
    ui.label_reportMissing->setText(t);

    return widget;
}

QStringList Plugin::appDirectories()
{
    return QStandardPaths::standardLocations(QStandardPaths::ApplicationsLocation);
}
