// Copyright (c) 2022-2023 Manuel Schneider

#include "plugin.h"
using albert::StandardItem;
using namespace std;

enum SupportedCommands {
    LOCK,
    LOGOUT,
    SUSPEND,
#if not defined(Q_OS_MAC)
    HIBERNATE,
#endif
    REBOOT,
    POWEROFF
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
        case REBOOT:    return R"R(osascript -e 'tell app "System Events" to restart')R";
        case POWEROFF:  return R"R(osascript -e 'tell app "System Events" to shut down')R";
    }
#endif
    return {};
}

Plugin::Plugin()
{
    auto s = settings();

    auto *si = &items_.emplace_back();
    si->item = StandardItem::make("lock", "Lock", "Lock the session", {"xdg:system-lock-screen", ":lock"});
    si->command = s->value(si->item->id(), defaultCommand(LOCK)).toString();
    si->aliases << si->item->id();

    si = &items_.emplace_back();
    si->item = StandardItem::make("logout", "Log out", "Quit the session", {"xdg:system-log-out", ":logout"});
    si->command = s->value(si->item->id(), defaultCommand(LOGOUT)).toString();
    si->aliases << si->item->id() << "logout" << "leave";

    si = &items_.emplace_back();
    si->item = StandardItem::make("suspend", "Suspend", "Suspend to memory", {"xdg:system-suspend", ":suspend"});
    si->command = s->value(si->item->id(), defaultCommand(SUSPEND)).toString();
    si->aliases << si->item->id() << "sleep";

#if not defined(Q_OS_MAC)
    si = &items_.emplace_back();
    si->item = StandardItem::make("hibernate", "Hibernate", "Suspend to disk", {"xdg:system-suspend-hibernate", ":hibernate"});
    si->command = s->value(si->item->id(), defaultCommand(HIBERNATE)).toString();
    si->aliases << si->item->id() << "suspend";
#endif

    si = &items_.emplace_back();
    si->item = StandardItem::make("reboot", "Restart", "Restart the machine", {"xdg:system-reboot", ":reboot"});
    si->command = s->value(si->item->id(), defaultCommand(REBOOT)).toString();
    si->aliases << si->item->id() << "restart";

    si = &items_.emplace_back();
    si->item = StandardItem::make("shutdown", "Shut down", "Shut down the machine", {"xdg:system-shutdown", ":poweroff"});
    si->command = s->value(si->item->id(), defaultCommand(POWEROFF)).toString();
    si->aliases << si->item->id() << "shut down" << "shutdown" << "poweroff" << "halt";

    for (auto &lsi : items_)
        lsi.item->setActions({{lsi.item->id(), lsi.item->text(),
                               [&](){ albert::runDetachedProcess({"/bin/sh", "-c", lsi.command}); }}});
}

#include <QFormLayout>
#include <QWidget>
#include <QLabel>
#include <QLineEdit>
QWidget* Plugin::buildConfigWidget()
{
    auto *w = new QWidget;
    auto *formLayout = new QFormLayout(w);
    formLayout->setFieldGrowthPolicy(QFormLayout::AllNonFixedFieldsGrow);
    for (auto &sys_item : items_){
        auto *label = new QLabel(sys_item.item->text(), w);
        auto *line_edit = new QLineEdit(sys_item.command, w);
        formLayout->addRow(label, line_edit);
        connect(line_edit, &QLineEdit::editingFinished, this, [this, line_edit, &sys_item]() {
            sys_item.command = line_edit->text();
            settings()->setValue(sys_item.item->id(), sys_item.command);
        });
    }
    return w;
}

void Plugin::updateIndexItems()
{
    std::vector<albert::IndexItem> index_items;
    for (const auto &si : items_)
        for (const auto &alias : si.aliases)
            index_items.emplace_back(si.item, alias);
    setIndexItems(::move(index_items));
}
