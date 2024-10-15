// Copyright (c) 2022-2024 Manuel Schneider

#include "application.h"
#include "plugin.h"
#include "terminal.h"
#include "ui_configwidget.h"
#include <QRegularExpression>
#include <QStandardPaths>
#include <QWidget>
#include <ranges>
using namespace std;
using namespace albert;

static const map<QString, QStringList> exec_args  // Desktop id > ExecArg
{
    {"alacritty", {"-e"}},
    {"app.devsuite.ptyxis", {"--"}},  // Flatpak
    {"blackbox", {"--"}},
    {"com.alacritty.Alacritty", {"-e"}},
    {"com.gexperts.tilix", {"-e"}},
    {"com.raggesilver.blackbox", {"--"}},  // Flatpak
    {"console", {"-e"}},
    {"contour", {"execute"}},
    {"cool-retro-term", {"-e"}},
    {"debian-xterm", {"-e"}},
    {"deepin-terminal", {"-e"}},
    {"deepin-terminal-gtk", {"-e"}},
    {"elementary-terminal", {"-x"}},
    {"kitty", {"--"}},
    {"konsole", {"-e"}},
    {"lxterminal", {"-e"}},
    {"mate-terminal", {"-x"}},
    {"org.codeberg.dnkl.foot", {}},
    {"org.contourterminal.contour", {"--"}},  // Flatpak
    {"org.gnome.console", {"-e"}},
    {"org.gnome.terminal", {"--"}},
    {"org.kde.konsole", {"-e"}},  // Flatpak
    {"org.wezfurlong.wezterm", {"-e"}},  // Flatpak
    {"ptyxis", {"--"}},
    {"qterminal", {"-e"}},
    {"roxterm", {"-x"}},
    {"st", {"-e"}},
    {"terminator", {"-u", "-x"}},  // https://github.com/gnome-terminator/terminator/issues/939
    {"terminology", {"-e"}},
    {"termite", {"-e"}},
    {"tilix", {"-e"}},
    {"urxvt", {"-e"}},
    {"uxterm", {"-e"}},
    {"wezterm", {"-e"}},
    {"xfce-terminal", {"-x"}},
    {"xfce4-terminal", {"-x"}},
    {"xterm", {"-e"}},
};

static QStringList appDirectories()
{ return QStandardPaths::standardLocations(QStandardPaths::ApplicationsLocation); }

Plugin* plugin = nullptr;

Plugin::Plugin()
{
    qunsetenv("DESKTOP_AUTOSTART_ID");
    plugin = this;

    fs_watcher.addPaths(appDirectories());
    connect(&fs_watcher, &QFileSystemWatcher::directoryChanged, this, &Plugin::updateIndexItems);


    // Load settings

    auto s = settings();

    restore_ignore_show_in_keys(s);
    connect(this, &Plugin::ignore_show_in_keys_changed,
            this, &Plugin::updateIndexItems);

    restore_use_exec(s);
    connect(this, &Plugin::use_exec_changed,
            this, &Plugin::updateIndexItems);

    restore_use_generic_name(s);
    connect(this, &Plugin::use_generic_name_changed,
            this, &Plugin::updateIndexItems);

    restore_use_keywords(s);
    connect(this, &Plugin::use_keywords_changed,
            this, &Plugin::updateIndexItems);

    restore_use_non_localized_name(s);
    connect(this, &PluginBase::use_non_localized_name_changed,
            this, &Plugin::updateIndexItems);


    // File watches

    for (const auto &path : QStandardPaths::standardLocations(QStandardPaths::ApplicationsLocation))
        for (auto dit = QDirIterator(path, QDir::Dirs|QDir::NoDotDot, QDirIterator::Subdirectories); dit.hasNext();)
            fs_watcher.addPath(QFileInfo(dit.next()).canonicalFilePath());

    connect(&fs_watcher, &QFileSystemWatcher::directoryChanged,
            this, [this](){ indexer.run(); });


    // Indexer

    indexer.parallel = [this](const bool &abort) -> vector<shared_ptr<applications::Application>>
    {

        Application::ParseOptions po {
            .ignore_show_in_keys = ignore_show_in_keys(),
            .use_exec = use_exec(),
            .use_generic_name = use_generic_name(),
            .use_keywords = use_keywords(),
            .use_non_localized_name = use_non_localized_name()
        };

        // Get a map of unique desktop entries according to the spec
        map<QString, shared_ptr<applications::Application>> apps;  // Desktop id > path
        for (const QString &dir : appDirectories())
        {
            DEBG << "Scanning desktop entries in:" << dir;

            QDirIterator fIt(dir, QStringList("*.desktop"), QDir::Files,
                             QDirIterator::Subdirectories|QDirIterator::FollowSymlinks);

            while (!fIt.next().isEmpty())
            {
                if (abort)
                    return {};

                const auto path = fIt.filePath();

                // To determine the ID of a desktop file, make its full path relative to
                // the $XDG_DATA_DIRS component in which the desktop file is installed,
                // remove the "applications/" prefix, and turn '/' into '-'.
                static const QRegularExpression re("^.*applications/");
                const auto id = fIt.filePath().remove(re).replace("/","-").chopped(8).toLower();  // '.desktop'

                try
                {
                    if (const auto &[it, success] = apps.emplace(id, make_shared<Application>(id, path, po));
                            success)
                        DEBG << QString("Valid desktop file '%1': '%2'").arg(path, it->second->name());
                    else
                        DEBG << QString("Skipped %1: Shadowed by '%2'").arg(path, it->second->path());
                }
                catch (const exception &e)
                {
                    DEBG << QString("Skipped %1: %2").arg(path, e.what());
                }
            }
        }

        vector<shared_ptr<applications::Application>> ret;
        ranges::move(apps | ranges::views::values, back_inserter(ret));
        return ret;
    };

    indexer.finish = [this](vector<shared_ptr<applications::Application>> &&result)
    {
        applications = ::move(result);

        INFO << QStringLiteral("Indexed %1 applications [%2 ms]")
                    .arg(applications.size()).arg(indexer.runtime.count());

        // Replace terminal apps with terminals and populate terminals
        // Filter supported terms by availability using destkop id
        terminals.clear();
        for (auto &app : applications)
            if (auto it = exec_args.find(app->id()); it != exec_args.cend())
            {
                auto term = make_shared<Terminal>(*static_cast<::Application*>(app.get()), it->second);
                app = static_pointer_cast<::Application>(term);
                terminals.emplace_back(term.get());
            }

        setUserTerminalFromConfig();

        setIndexItems(buildIndexItems());

    };
}

Plugin::~Plugin() = default;

QWidget *Plugin::buildConfigWidget()
{
    auto widget = new QWidget;
    Ui::ConfigWidget ui;
    ui.setupUi(widget);

    ALBERT_PROPERTY_CONNECT_CHECKBOX(this, ignore_show_in_keys, ui.checkBox_ignoreShowInKeys);
    ALBERT_PROPERTY_CONNECT_CHECKBOX(this, use_exec, ui.checkBox_useExec);
    ALBERT_PROPERTY_CONNECT_CHECKBOX(this, use_generic_name, ui.checkBox_useGenericName);
    ALBERT_PROPERTY_CONNECT_CHECKBOX(this, use_keywords, ui.checkBox_useKeywords);

    addBaseConfig(ui.formLayout);

    return widget;
}

void Plugin::runTerminal(QStringList commandline, const QString working_dir) const
{
    terminal->launch(commandline, working_dir);
}
