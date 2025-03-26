// Copyright (c) 2017-2024 Manuel Schneider

#include "plugin.h"
#include "ui_configwidget.h"
#include <QCheckBox>
#include <QLineEdit>
#include <QSettings>
#include <albert/albert.h>
#include <albert/extensionregistry.h>
#include <albert/logging.h>
#include <albert/standarditem.h>
ALBERT_LOGGING_CATEGORY("system")
using namespace albert;
using namespace std;

static QString defaultCommand(SupportedCommands command)
{
#if defined(Q_OS_MAC)
    switch (command) {
        case LOCK:      return R"R(pmset displaysleepnow)R";
        case LOGOUT:    return R"R(osascript -e 'tell app "System Events" to log out')R";
        case SUSPEND:   return R"R(osascript -e 'tell app "System Events" to sleep')R";
        case REBOOT:    return R"R(osascript -e 'tell app "System Events" to restart')R";
        case POWEROFF:  return R"R(osascript -e 'tell app "System Events" to shut down')R";
    }
#elif defined(Q_OS_UNIX)
    for (const QString &de : QString(::getenv("XDG_CURRENT_DESKTOP")).split(":")) {

        if (de == "Unity" || de == "Pantheon" || de == "GNOME")
            switch (command) {
            case LOCK:      return "dbus-send --type=method_call --dest=org.gnome.ScreenSaver /org/gnome/ScreenSaver org.gnome.ScreenSaver.Lock";
            case LOGOUT:    return "gnome-session-quit --logout --no-prompt";
            case SUSPEND:   break ;
            case HIBERNATE: break ;
            case REBOOT:    return "gnome-session-quit --reboot --no-prompt";
            case POWEROFF:  return "gnome-session-quit --power-off --no-prompt";
        }

        else if (de == "kde-plasma" || de == "KDE")
            switch (command) {
            case LOCK:      return "dbus-send --dest=org.freedesktop.ScreenSaver --type=method_call /ScreenSaver org.freedesktop.ScreenSaver.Lock";
            case LOGOUT:    return "qdbus org.kde.Shutdown /Shutdown  org.kde.Shutdown.logout";
            case SUSPEND:   break ;
            case HIBERNATE: break ;
            case REBOOT:    return "qdbus org.kde.Shutdown /Shutdown  org.kde.Shutdown.logoutAndReboot";
            case POWEROFF:  return "qdbus org.kde.Shutdown /Shutdown  org.kde.Shutdown.logoutAndShutdown";
        }

        else if (de == "X-Cinnamon" || de == "Cinnamon")
            switch (command) {
            case LOCK:      return "cinnamon-screensaver-command --lock";
            case LOGOUT:    return "cinnamon-session-quit --logout";
            case SUSPEND:   break ;
            case HIBERNATE: break ;
            case REBOOT:    return "cinnamon-session-quit --reboot";
            case POWEROFF:  return "cinnamon-session-quit --power-off";
        }

        else if (de == "MATE")
            switch (command) {
            case LOCK:      return "mate-screensaver-command --lock";
            case LOGOUT:    return "mate-session-save --logout-dialog";
            case SUSPEND:   return "sh -c \"mate-screensaver-command --lock && systemctl suspend -i\"";
            case HIBERNATE: return "sh -c \"mate-screensaver-command --lock && systemctl hibernate -i\"";
            case REBOOT:    return "mate-session-save --shutdown-dialog";
            case POWEROFF:  return "mate-session-save --shutdown-dialog";
        }

        else if (de == "XFCE")
            switch (command) {
            case LOCK:      return "xflock4";
            case LOGOUT:    return "xfce4-session-logout --logout";
            case SUSPEND:   return "xfce4-session-logout --suspend";
            case HIBERNATE: return "xfce4-session-logout --hibernate";
            case REBOOT:    return "xfce4-session-logout --reboot";
            case POWEROFF:  return "xfce4-session-logout --halt";
        }

        else if (de == "LXQt")
            switch (command) {
            case LOCK:      return "lxqt-leave --lockscreen";
            case LOGOUT:    return "lxqt-leave --logout";
            case SUSPEND:   return "lxqt-leave --suspend";
            case HIBERNATE: return "lxqt-leave --hibernate";
            case REBOOT:    return "lxqt-leave --reboot";
            case POWEROFF:  return "lxqt-leave --shutdown";
        }
    }
    switch (command) {
    case LOCK:      return "xdg-screensaver lock";
    case LOGOUT:    return "notify-send \"Error.\" \"Logout command is not set.\" --icon=system-log-out";
    case SUSPEND:   return "systemctl suspend -i";
    case HIBERNATE: return "systemctl hibernate -i";
    case REBOOT:    return "notify-send \"Error.\" \"Reboot command is not set.\" --icon=system-reboot";
    case POWEROFF:  return "notify-send \"Error.\" \"Poweroff command is not set.\" --icon=system-shutdown";
    } 
#endif
    return {};
}

Plugin::Plugin():
    commands{
        {
            .id = LOCK,
            .config_key_enabled = "lock_enabled",
            .config_key_title = "title_lock",
            .config_key_command = "command_lock",
            .icon_urls = {"xdg:system-lock-screen", ":lock"},
            .default_title = tr("Lock"),
            .description = tr("Lock the session"),
            .command = defaultCommand(LOCK),
        },
        {
            .id = LOGOUT,
            .config_key_enabled = "logout_enabled",
            .config_key_title = "title_logout",
            .config_key_command = "command_logout",
            .icon_urls = {"xdg:system-log-out", ":logout"},
            .default_title = tr("Logout"),
            .description = tr("Quit the session"),
            .command = defaultCommand(LOGOUT),
        },
        {
            .id = SUSPEND,
            .config_key_enabled = "suspend_enabled",
            .config_key_title = "title_suspend",
            .config_key_command = "command_suspend",
            .icon_urls = {"xdg:system-suspend", ":suspend"},
            .default_title = tr("Suspend"),
            .description = tr("Suspend to memory"),
            .command = defaultCommand(SUSPEND),
        },
#if not defined(Q_OS_MAC)
        {
            .id = HIBERNATE,
            .config_key_enabled = "hibernate_enabled",
            .config_key_title = "title_hibernate",
            .config_key_command = "command_hibernate",
            .icon_urls = {"xdg:system-suspend-hibernate", ":hibernate"},
            .default_title = tr("Hibernate"),
            .description = tr("Suspend to disk"),
            .command = defaultCommand(HIBERNATE),
        },
#endif
        {
            .id = REBOOT,
            .config_key_enabled = "reboot_enabled",
            .config_key_title = "title_reboot",
            .config_key_command = "command_reboot",
            .icon_urls = {"xdg:system-reboot", ":reboot"},
            .default_title = tr("Reboot"),
            .description = tr("Restart the machine"),
            .command = defaultCommand(REBOOT),
        },
        {
            .id = POWEROFF,
            .config_key_enabled = "poweroff_enabled",
            .config_key_title = "title_poweroff",
            .config_key_command = "command_poweroff",
            .icon_urls = {"xdg:system-shutdown", ":poweroff"},
            .default_title = tr("Poweroff"),
            .description = tr("Shut down the machine"),
            .command = defaultCommand(POWEROFF),
        }
    }
{
}


QWidget* Plugin::buildConfigWidget()
{
    auto *w = new QWidget;
    Ui::ConfigWidget ui;
    ui.setupUi(w);

    auto s = settings();
    for (uint i = 0; i < commands.size(); ++i)
    {
        const auto &c = commands[i];

        auto *checkbox = new QCheckBox(w);
        auto *label = new QLabel(c.description, w);
        auto *line_edit_title = new QLineEdit(w);
        auto *line_edit_command = new QLineEdit(w);

        bool enabled = s->value(c.config_key_enabled, true).toBool();

        checkbox->setCheckState(enabled ? Qt::CheckState::Checked : Qt::CheckState::Unchecked);
        connect(checkbox, &QCheckBox::clicked, this, [=, this](bool checked)
        {
            settings()->setValue(c.config_key_enabled, checked);

            // Restore defaults if unchecked
            if (!checked){
                settings()->remove(c.config_key_title);
                settings()->remove(c.config_key_command);
                line_edit_title->clear();
                line_edit_command->clear();
            }

            label->setEnabled(checked);
            line_edit_title->setEnabled(checked);
            line_edit_command->setEnabled(checked);

            updateIndexItems();
        });

        label->setEnabled(enabled);

        line_edit_title->setEnabled(enabled);
        // line_edit_title->setClearButtonEnabled(true);
        line_edit_title->setFixedWidth(100);
        line_edit_title->setPlaceholderText(c.default_title);
        line_edit_title->setText(s->value(c.config_key_title).toString());
        connect(line_edit_title, &QLineEdit::editingFinished,
                this, [this, line_edit_title, ck=c.config_key_title]
        {
            if (line_edit_title->text().isEmpty())
                settings()->remove(ck);
            else
                settings()->setValue(ck, line_edit_title->text());
            updateIndexItems();
        });

        line_edit_command->setEnabled(enabled);
        // line_edit_command->setClearButtonEnabled(true);
        line_edit_command->setPlaceholderText(defaultCommand(c.id));
        line_edit_command->setText(s->value(c.config_key_command).toString());
        connect(line_edit_command, &QLineEdit::editingFinished,
                this, [this, line_edit_command, ck=c.config_key_command]
        {
            if (line_edit_command->text().isEmpty())
                settings()->remove(ck);
            else
                settings()->setValue(ck, line_edit_command->text());
            updateIndexItems();
        });

        ui.gridLayout_commands->addWidget(checkbox, i * 2, 0);
        ui.gridLayout_commands->addWidget(label, i * 2, 1, 1, 2);
        ui.gridLayout_commands->addWidget(line_edit_title, i * 2 + 1, 1);
        ui.gridLayout_commands->addWidget(line_edit_command, i * 2 + 1, 2);
    }

    ui.verticalLayout->addStretch();

    return w;
}

void Plugin::updateIndexItems()
{
    vector<albert::IndexItem> index_items;
    auto s = settings();

    for (const auto &c : commands)
    {
        // skip if disabled
        if (!s->value(c.config_key_enabled, true).toBool())
            continue;

        auto item = StandardItem::make(
            c.default_title,
            settings()->value(c.config_key_title, c.default_title).toString(),
            c.description,
            c.icon_urls,
            {
                {
                    c.default_title, c.description,
                    [this, &c](){ albert::runDetachedProcess({
                        "/bin/sh", "-c",
                        settings()->value(c.config_key_command, defaultCommand(c.id)).toString()});
                    }
                }
            }
        );

        index_items.emplace_back(::move(item), item->text());
    }

    setIndexItems(::move(index_items));
}
