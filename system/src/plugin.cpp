// Copyright (c) 2022-2023 Manuel Schneider

#include "plugin.h"
#include <QCheckBox>
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QSpacerItem>
#include <QWidget>
using albert::StandardItem;
using namespace std;

enum SupportedCommands {
    LOCK,
    LOGOUT,
    SUSPEND,
    HIBERNATE,
    REBOOT,
    POWEROFF
};

static const constexpr char *config_keys_enabled[] = {
    "lock_enabled",
    "logout_enabled",
    "suspend_enabled",
    "hibernate_enabled",
    "reboot_enabled",
    "poweroff_enabled"
};

static const constexpr char *config_keys_title[] = {
    "title_lock",
    "title_logout",
    "title_suspend",
    "title_hibernate",
    "title_reboot",
    "title_poweroff"
};

static const constexpr char *config_keys_command[] = {
    "command_lock",
    "command_logout",
    "command_suspend",
    "command_hibernate",
    "command_reboot",
    "command_poweroff"
};

static const constexpr char *default_title[] = {
    "Lock",
    "Logout",
    "Suspend",
    "Hibernate",
    "Reboot",
    "Poweroff"
};

static const constexpr char *descriptions[] = {
    "Lock the session",
    "Quit the session",
    "Suspend to memory",
    "Suspend to disk",
    "Restart the machine",
    "Shut down the machine"
};

static const QStringList icon_urls[] = {
    {"xdg:system-lock-screen", ":lock"},
    {"xdg:system-log-out", ":logout"},
    {"xdg:system-suspend", ":suspend"},
    {"xdg:system-suspend-hibernate", ":hibernate"},
    {"xdg:system-reboot", ":reboot"},
    {"xdg:system-shutdown", ":poweroff"}
};

static QString defaultCommand(SupportedCommands command)
{
#if defined(Q_OS_LINUX)
    for (const QString &de : QString(::getenv("XDG_CURRENT_DESKTOP")).split(":")) {

        if (de == "Unity" || de == "Pantheon" || de == "GNOME")
            switch (command) {
            case LOCK:      return "dbus-send --type=method_call --dest=org.gnome.ScreenSaver /org/gnome/ScreenSaver org.gnome.ScreenSaver.Lock";
            case LOGOUT:    return "gnome-session-quit --logout";
            case SUSPEND:   break ;
            case HIBERNATE: break ;
            case REBOOT:    return "gnome-session-quit --reboot";
            case POWEROFF:  return "gnome-session-quit --power-off";
        }

        else if (de == "kde-plasma" || de == "KDE")
            switch (command) {
            case LOCK:      return "dbus-send --dest=org.freedesktop.ScreenSaver --type=method_call /ScreenSaver org.freedesktop.ScreenSaver.Lock";
            case LOGOUT:    return "qdbus org.kde.ksmserver /KSMServer logout 0 0 0";
            case SUSPEND:   break ;
            case HIBERNATE: break ;
            case REBOOT:    return "qdbus org.kde.ksmserver /KSMServer logout 0 1 0";
            case POWEROFF:  return "qdbus org.kde.ksmserver /KSMServer logout 0 2 0";
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
#elif defined(Q_OS_MAC)
    switch (command) {
        case LOCK:      return R"R(pmset displaysleepnow)R";
        case LOGOUT:    return R"R(osascript -e 'tell app "System Events" to  «event aevtrlgo»')R";
        case SUSPEND:   return R"R(osascript -e 'tell app "System Events" to sleep')R";
        case HIBERNATE: throw runtime_error("HIBERNATE not supported on MacOS");
        case REBOOT:    return R"R(osascript -e 'tell app "System Events" to restart')R";
        case POWEROFF:  return R"R(osascript -e 'tell app "System Events" to shut down')R";
    }
#endif
    return {};
}

QWidget* Plugin::buildConfigWidget()
{
    auto s = settings();
    auto *w = new QWidget;
    auto *l = new QGridLayout(w);
    w->setLayout(l);



    auto *infoLabel = new QLabel("Disabling restores default values. If your are "
                                 "missing sensible default values for your system "
                                 "leave a note on the GitHub issue tracker.");
    infoLabel->setWordWrap(true);
    int row = 0;
    l->addWidget(infoLabel, row++, 0, 1, 3);
    for (SupportedCommands action : {
             LOCK,
             LOGOUT,
             SUSPEND,
#if not defined(Q_OS_MAC)
             HIBERNATE,
#endif
             REBOOT,
             POWEROFF}){

        auto *checkbox = new QCheckBox(w);
        auto *label = new QLabel(descriptions[action], w);
        auto *line_edit_title = new QLineEdit(w);
        auto *line_edit_command = new QLineEdit(w);

        bool enabled = s->value(config_keys_enabled[action], true).toBool();

        checkbox->setCheckState(enabled ? Qt::CheckState::Checked : Qt::CheckState::Unchecked);
        connect(checkbox, &QCheckBox::clicked, this, [=, this](bool checked) {
            settings()->setValue(config_keys_enabled[action], checked);

            // Restore defaults if unchecked
            if (!checked){
                settings()->remove(config_keys_title[action]);
                settings()->remove(config_keys_command[action]);
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
        line_edit_title->setFixedWidth(100);
        line_edit_title->setPlaceholderText(default_title[action]);
        line_edit_title->setText(s->value(config_keys_title[action]).toString());
        connect(line_edit_title, &QLineEdit::editingFinished, this, [this, line_edit_title, action]() {
            if (line_edit_title->text().isEmpty())
                settings()->remove(config_keys_title[action]);
            else
                settings()->setValue(config_keys_title[action], line_edit_title->text());
            updateIndexItems();
        });

        line_edit_command->setEnabled(enabled);
        line_edit_command->setPlaceholderText(defaultCommand(action));
        line_edit_command->setText(s->value(config_keys_command[action]).toString());
        connect(line_edit_command, &QLineEdit::editingFinished, this, [this, line_edit_command, action]() {
            if (line_edit_command->text().isEmpty())
                settings()->remove(config_keys_command[action]);
            else
                settings()->setValue(config_keys_command[action], line_edit_command->text());
            updateIndexItems();
        });

        l->addWidget(checkbox, row*2, 0);
        l->addWidget(label, row*2, 1, 1, 2);
        l->addWidget(line_edit_title, row*2+1, 1);
        l->addWidget(line_edit_command, row*2+1, 2);

        ++row;
    }
    l->addItem(new QSpacerItem(0,0,QSizePolicy::Expanding,QSizePolicy::Expanding), row*2, 0, 1, 3);
    l->setColumnStretch(1,2);  // strech last column
    l->setContentsMargins(0,0,0,0);
    return w;
}

void Plugin::updateIndexItems()
{
    std::vector<albert::IndexItem> index_items;
    auto s = settings();

    for (SupportedCommands action : {
             LOCK,
                 LOGOUT,
                 SUSPEND,
#if not defined(Q_OS_MAC)
                 HIBERNATE,
#endif
                 REBOOT,
                 POWEROFF}){

        // skip if disabled
        if (!s->value(config_keys_enabled[action], true).toBool())
            continue;

        auto item = StandardItem::make(
            default_title[action],
            settings()->value(config_keys_title[action], default_title[action]).toString(),
            descriptions[action],
            icon_urls[action],
            {
                {
                    default_title[action], descriptions[action],
                    [this, action](){ albert::runDetachedProcess({
                        "/bin/sh", "-c",
                        settings()->value(config_keys_command[action], defaultCommand(action)).toString()});
                    }
                }
            }
        );

        index_items.emplace_back(::move(item), item->text());
    }

    setIndexItems(::move(index_items));
}
