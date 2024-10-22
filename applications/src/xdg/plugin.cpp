// Copyright (c) 2022-2024 Manuel Schneider

#include "application.h"
#include "plugin.h"
#include "terminal.h"
#include "ui_configwidget.h"
#include <QRegularExpression>
#include <QStandardPaths>
#include <QWidget>
using namespace std;
using namespace albert;

const map<QString, QStringList> Plugin::exec_args  // Desktop id > ExecArg
{
    {"Alacritty", {"-e"}},
    {"app.devsuite.Ptyxis", {"--"}},  // Flatpak
    {"blackbox", {"--"}},
    {"com.alacritty.Alacritty", {"-e"}},
    {"com.gexperts.tilix", {"-e"}},
    {"com.raggesilver.BlackBox", {"--"}},  // Flatpak
    {"Console", {"-e"}},
    {"Contour", {"execute"}},
    {"cool-retro-term", {"-e"}},
    {"debian-uxterm", {"-e"}},
    {"debian-xterm", {"-e"}},
    {"deepin-terminal", {"-e"}},
    {"deepin-terminal-gtk", {"-e"}},
    {"elementary-terminal", {"-x"}},
    {"kitty", {"--"}},
    {"konsole", {"-e"}},
    {"lxterminal", {"-e"}},
    {"mate-terminal", {"-x"}},
    {"org.codeberg.dnkl.foot", {}},
    {"org.contourterminal.Contour", {"--"}},  // Flatpak
    {"org.gnome.Console", {"-e"}},
    {"org.gnome.ptyxis", {"--"}},
    {"org.gnome.Terminal", {"--"}},
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
        // Get a map of unique desktop entries according to the spec

        map<QString, QString> desktop_files;  // Desktop id > path
        for (const QString &dir : appDirectories())
        {
            DEBG << "Scanning desktop entries in:" << dir;

            QDirIterator it(dir, QStringList("*.desktop"), QDir::Files,
                            QDirIterator::Subdirectories|QDirIterator::FollowSymlinks);

            while (it.hasNext())
            {
                auto path = it.next();

                // To determine the ID of a desktop file, make its full path relative to
                // the $XDG_DATA_DIRS component in which the desktop file is installed,
                // remove the "applications/" prefix, and turn '/' into '-'. Chop off '.desktop'.
                static QRegularExpression re("^.*applications/");
                QString id = QString(path).remove(re).replace("/","-").chopped(8);

                if (const auto &[dit, success] = desktop_files.emplace(id, path); !success)
                    DEBG << QString("Desktop file '%1' at '%2' will be skipped: Shadowed by '%3'")
                                .arg(id, path, desktop_files[id]);
            }
        }

        Application::ParseOptions po{
            .ignore_show_in_keys = ignore_show_in_keys(),
            .use_exec = use_exec(),
            .use_generic_name = use_generic_name(),
            .use_keywords = use_keywords(),
            .use_non_localized_name = use_non_localized_name()
        };

        // Index the unique desktop files
        vector<shared_ptr<applications::Application>> apps;
        for (const auto &[id, path] : desktop_files)
        {
            if (abort)
                return apps;

            try
            {
                apps.emplace_back(make_shared<Application>(id, path, po));
                DEBG << QString("Valid desktop file '%1': '%2'").arg(id, path);
            }
            catch (const exception &e)
            {
                DEBG << QString("Skipped desktop entry '%1': %2").arg(path, e.what());
            }
        }

        return apps;
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

        emit appsChanged();
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

QJsonObject Plugin::telemetryData() const
{
    QJsonObject t;
    for (const auto &iapp : applications)
        if (const auto &app = static_pointer_cast<::Application>(iapp); app->isTerminal())
            t.insert(app->id(), app->exec().join(" "));

    QJsonObject o;
    o.insert("terminals", t);
    return o;
}
