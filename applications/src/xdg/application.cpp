// Copyright (c) 2022-2024 Manuel Schneider

#include "application.h"
#include "desktopentryparser.h"
#include "plugin.h"
#include <albert/util.h>
using namespace std;
using namespace albert;

extern Plugin* plugin;

Application::Application(const QString &id, const QString &path, ParseOptions po)
{
    id_ = id;
    path_ = path;

    DesktopEntryParser p(path);
    auto root_section = QStringLiteral("Desktop Entry");

    // Type - string, REQUIRED to be Application
    if (p.getString(root_section, QStringLiteral("Type")) != QStringLiteral("Application"))
        throw runtime_error("Desktop entries of type other than 'Application' are not handled yet.");

    // NoDisplay - boolean, must not be true
    try {
        if (p.getBoolean(root_section, QStringLiteral("NoDisplay")))
            exclude_reason_ = ExcludeReason::NoDisplay;
    } catch (const out_of_range &) { }

    if (!po.ignore_show_in_keys && exclude_reason_ == ExcludeReason::None)
    {
        const auto desktops(QString(getenv("XDG_CURRENT_DESKTOP")).split(':', Qt::SkipEmptyParts));

        // NotShowIn - string(s), if exists must not be in XDG_CURRENT_DESKTOP
        try {
            if (ranges::any_of(p.getString(root_section, QStringLiteral("NotShowIn")).split(';', Qt::SkipEmptyParts),
                               [&](const auto &de){ return desktops.contains(de); }))
                exclude_reason_ = ExcludeReason::NotShowIn;
        } catch (const out_of_range &) { }

        // OnlyShowIn - string(s), if exists has to be in XDG_CURRENT_DESKTOP
        try {
            if (!ranges::any_of(p.getString(root_section, QStringLiteral("OnlyShowIn")).split(';', Qt::SkipEmptyParts),
                                [&](const auto &de){ return desktops.contains(de); }))
                exclude_reason_ = ExcludeReason::OnlyShowIn;
        } catch (const out_of_range &) { }
    }

    // Localized name - localestring, may equal name if no localizations available
    // No need to catch despite optional, since falls back to name, which is required
    names_ << p.getLocaleString(root_section, QStringLiteral("Name"));

    // Non localized name - string, REQUIRED
    if (po.use_non_localized_name)
        names_ << p.getString(root_section, QStringLiteral("Name"));

    // Exec - string
    try
    {
        exec_ = DesktopEntryParser::splitExec(p.getString(root_section, QStringLiteral("Exec"))).value();
    }
    catch (const out_of_range &) { }
    catch (const bad_optional_access&)
    {
        throw runtime_error("Malformed Exec value.");
    }
    if (exec_.isEmpty() && exclude_reason_ == ExcludeReason::None)
        exclude_reason_ = ExcludeReason::EmptyExec;

    if (po.use_exec && !exec_.isEmpty())
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

        if (ranges::none_of(excludes, [this](const QString &str){ return exec_.startsWith(str); }))
            names_ << exec_.at(0);
    }

    // Comment - localestring
    try {
        description_ = p.getLocaleString(root_section, QStringLiteral("Comment"));
    } catch (const out_of_range &) { }

    // Keywords - localestring(s)
    try {
        auto keywords = p.getLocaleString(root_section, QStringLiteral("Keywords")).split(';', Qt::SkipEmptyParts);
        if (description_.isEmpty())
            description_ = keywords.join(", ");
        if (po.use_keywords)
            names_ << keywords;
    } catch (const out_of_range &) { }

    // Icon - iconstring (xdg icon naming spec)
    try {
        icon_ = p.getLocaleString(root_section, QStringLiteral("Icon"));
    } catch (const out_of_range &) { }

    // Path - string
    try {
        working_dir_ = p.getString(root_section, QStringLiteral("Path"));
    } catch (const out_of_range &) { }

    // Terminal - boolean
    try {
        term_ = p.getBoolean(root_section, QStringLiteral("Terminal"));
    } catch (const out_of_range &) { }

    // GenericName - localestring
    if (po.use_generic_name)
        try {
            names_<< p.getLocaleString(root_section, QStringLiteral("GenericName"));
        }
        catch (const out_of_range &) { }

    // Actions - string(s)
    try {
        auto action_ids = p.getString(root_section, QStringLiteral("Actions")).split(';', Qt::SkipEmptyParts);
        for (const QString &action_id : action_ids)
        {
            try
            {
                const auto action_section = QString("Desktop Action %1").arg(action_id);

                // TOdo
                auto action = Action(
                    action_id,
                    p.getLocaleString(action_section, QStringLiteral("Name")), // Name - localestring, REQUIRED
                    [this, &p, &action_section]{
                        auto exec = DesktopEntryParser::splitExec(p.getString(action_section, QStringLiteral("Exec")));
                        if (!exec)
                            throw runtime_error("Malformed Exec value.");
                        else if (exec.value().isEmpty())
                            throw runtime_error("Empty Exec value.");
                        else
                            albert::runDetachedProcess(fieldCodesExpanded(exec.value(), QUrl()));
                    }
                );

                // Name - localestring, REQUIRED
                auto name = p.getLocaleString(action_section, QStringLiteral("Name"));

                // Exec - string, REQUIRED despite not strictly by standard
                auto exec = DesktopEntryParser::splitExec(p.getString(action_section, QStringLiteral("Exec")));
                if (!exec)
                    throw runtime_error("Malformed Exec value.");
                else if (exec.value().isEmpty())
                    throw runtime_error("Empty Exec value.");
                else
                    desktop_actions_.emplace_back(*this, action_id, name, exec.value());
            }
            catch (const out_of_range &e)
            {
                WARN << QString("%1: Desktop action '%2' skipped: %3").arg(path, action_id, e.what());
            }
        }
    } catch (const out_of_range &) { }

    // // MimeType, string(s)
    // try {
    //     pe.mime_types = p.getString(root_section, QStringLiteral("MimeType")).split(';', Qt::SkipEmptyParts);
    // } catch (const out_of_range &) { }
    // pe.mime_types.removeDuplicates();

    names_.removeDuplicates();
}

QString Application::subtext() const { return description_; }

QStringList Application::iconUrls() const
{
    if (QFileInfo(icon_).isAbsolute())
        return { icon_ };
    else
        return { QString("xdg:%1").arg(icon_) };
}

vector<Action> Application::actions() const
{
    vector<Action> actions = ApplicationBase::actions();

    for (const auto &a : desktop_actions_)
        actions.emplace_back(QString("action-%1").arg(a.id_), a.name_, [&a]{ a.launch(); });

    actions.emplace_back("reveal-entry",
                         Plugin::tr("Open desktop entry"),
                         [this](){ albert::openUrl(path_); });

    return actions;
}

const Application::ExcludeReason &Application::excludeReason() const
{
    return exclude_reason_;
}

const QStringList &Application::exec() const
{
    return exec_;
}

void Application::launchExec(const QStringList &exec, QUrl url, const QString &working_dir) const
{
    auto commandline = fieldCodesExpanded(exec, url);
    auto wd = working_dir.isEmpty() ? working_dir_ : working_dir;
    if (term_)
        plugin->runTerminal(commandline, wd);
    else
        albert::runDetachedProcess(commandline, wd);
}

void Application::launch() const { launchExec(exec_, {}, {}); }

void Application::DesktopAction::launch() const { application.launchExec(exec_, {}, {}); }

QStringList Application::fieldCodesExpanded(const QStringList &exec, QUrl url) const
{
    // TODO proper support for %f %F %U

    // Code	Description
    // %% : '%'
    // %f : A single file name (including the path), even if multiple files are selected. The system reading the desktop entry should recognize that the program in question cannot handle multiple file arguments, and it should should probably spawn and execute multiple copies of a program for each selected file if the program is not able to handle additional file arguments. If files are not on the local file system (i.e. are on HTTP or FTP locations), the files will be copied to the local file system and %f will be expanded to point at the temporary file. Used for programs that do not understand the URL syntax.
    // %F : A list of files. Use for apps that can open several local files at once. Each file is passed as a separate argument to the executable program.
    // %u : A single URL. Local files may either be passed as file: URLs or as file path.
    // %U : A list of URLs. Each URL is passed as a separate argument to the executable program. Local files may either be passed as file: URLs or as file path.
    // %i : The Icon key of the desktop entry expanded as two arguments, first --icon and then the value of the Icon key. Should not expand to any arguments if the Icon key is empty or missing.
    // %c : The translated name of the application as listed in the appropriate Name key in the desktop entry.
    // %k : The location of the desktop file as either a URI (if for example gotten from the vfolder system) or a local filename or empty if no location is known.
    // Deprecated: %v %m %d %D %n %N

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
        else if (t == "%i" && !icon_.isNull())
            c << "--icon" << icon_;
        else if (t == "%c")
            c << name();
        else if (t == "%k")
            c << path_;
        else if (t == "%v" || t == "%m" || t == "%d" || t == "%D" || t == "%n" || t == "%N")
            ;  // Skipping deprecated field codes
        else
            c << t;
    }
    return c;
}
