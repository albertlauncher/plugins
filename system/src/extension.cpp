// Copyright (C) 2014-2020 Manuel Schneider

#include <QPointer>
#include <QRegularExpression>
#include <QSettings>
#include <array>
#include "albert/util/standardactions.h"
#include "albert/util/standarditem.h"
#include "configwidget.h"
#include "extension.h"
#include "xdg/iconlookup.h"
Q_LOGGING_CATEGORY(qlc, "system")
#define DEBG qCDebug(qlc,).noquote()
#define INFO qCInfo(qlc,).noquote()
#define WARN qCWarning(qlc,).noquote()
#define CRIT qCCritical(qlc,).noquote()
using namespace std;
using namespace Core;


namespace {

const uint NUMCOMMANDS = 7;

enum SupportedCommands {
    LOCK,
    LOGOUT,
    SUSPEND,
    HIBERNATE,
    REBOOT,
    POWEROFF,
    SCREENOFF,
};

array<const QString, 7> configNames{{
    "lock",
    "logout",
    "suspend",
    "hibernate",
    "reboot",
    "shutdown",
    "screenoff"
}};

array<const QString, 7> itemTitles{{
    "Lock",
    "Log out",
    "Suspend",
    "Hibernate",
    "Restart",
    "Shut down",
    "Screen Off"
}};

array<vector<QString>, 7> aliases{{
    {"lock"},
    {"log out", "logout", "leave"},
    {"suspend", "sleep"},
    {"suspend", "hibernate"},
    {"restart", "reboot"},
    {"shut down", "shutdown", "poweroff", "halt"},
    {"screenoff","monitoroff","dark"}
}};

array<const QString, 7> itemDescriptions{{
    "Lock the session.",
    "Quit the session.",
    "Suspend to memory.",
    "Suspend to disk.",
    "Restart the machine.",
    "Shut down the machine.",
    "Turn Off the Monitor without Locking"
}};

array<const QString, 7> iconNames{{
    "system-lock-screen",
    "system-log-out",
    "system-suspend",
    "system-suspend-hibernate",
    "system-reboot",
    "system-shutdown",
    "system-suspend-hibernate"
}};


QString defaultCommand(SupportedCommands command){

    for (const QString &de : QString(::getenv("XDG_CURRENT_DESKTOP")).split(":")) {

        if (de == "Unity" || de == "Pantheon" || de == "GNOME")
            switch (command) {
            case LOCK:      return "dbus-send --type=method_call --dest=org.gnome.ScreenSaver /org/gnome/ScreenSaver org.gnome.ScreenSaver.Lock";
            case LOGOUT:    return "gnome-session-quit --logout";
            case SUSPEND:   break ;
            case HIBERNATE: break ;
            case REBOOT:    return "gnome-session-quit --reboot";
            case POWEROFF:  return "gnome-session-quit --power-off";
            case SCREENOFF:  return "xset dpms force off";
            }

        else if (de == "kde-plasma" || de == "KDE")
            switch (command) {
            case LOCK:      return "dbus-send --dest=org.freedesktop.ScreenSaver --type=method_call /ScreenSaver org.freedesktop.ScreenSaver.Lock";
            case LOGOUT:    return "qdbus org.kde.ksmserver /KSMServer logout 0 0 0";
            case SUSPEND:   break ;
            case HIBERNATE: break ;
            case REBOOT:    return "qdbus org.kde.ksmserver /KSMServer logout 0 1 0";
            case POWEROFF:  return "qdbus org.kde.ksmserver /KSMServer logout 0 2 0";
            case SCREENOFF:  return "xset dpms force off";
            }

        else if (de == "X-Cinnamon" || de == "Cinnamon")
            switch (command) {
            case LOCK:      return "cinnamon-screensaver-command --lock";
            case LOGOUT:    return "cinnamon-session-quit --logout";
            case SUSPEND:   break ;
            case HIBERNATE: break ;
            case REBOOT:    return "cinnamon-session-quit --reboot";
            case POWEROFF:  return "cinnamon-session-quit --power-off";
            case SCREENOFF:  return "xset dpms force off";
            }

        else if (de == "MATE")
            switch (command) {
            case LOCK:      return "mate-screensaver-command --lock";
            case LOGOUT:    return "mate-session-save --logout-dialog";
            case SUSPEND:   return "sh -c \"mate-screensaver-command --lock && systemctl suspend -i\"";
            case HIBERNATE: return "sh -c \"mate-screensaver-command --lock && systemctl hibernate -i\"";
            case REBOOT:    return "mate-session-save --shutdown-dialog";
            case POWEROFF:  return "mate-session-save --shutdown-dialog";
            case SCREENOFF:  return "xset dpms force off";
            }

        else if (de == "XFCE")
            switch (command) {
            case LOCK:      return "xflock4";
            case LOGOUT:    return "xfce4-session-logout --logout";
            case SUSPEND:   return "xfce4-session-logout --suspend";
            case HIBERNATE: return "xfce4-session-logout --hibernate";
            case REBOOT:    return "xfce4-session-logout --reboot";
            case POWEROFF:  return "xfce4-session-logout --halt";
            case SCREENOFF:  return "xset dpms force off";
            }
    }

    switch (command) {
    case LOCK:      return "xdg-screensaver lock";
    case LOGOUT:    return "notify-send \"Error.\" \"Logout command is not set.\" --icon=system-log-out";
    case SUSPEND:   return "systemctl suspend -i";
    case HIBERNATE: return "systemctl hibernate -i";
    case REBOOT:    return "notify-send \"Error.\" \"Reboot command is not set.\" --icon=system-reboot";
    case POWEROFF:  return "notify-send \"Error.\" \"Poweroff command is not set.\" --icon=system-shutdown";
    case SCREENOFF:  return "xset dpms force off";
    }

    // NEVER REACHED
    return QString();
}

}



class System::Private
{
public:
    QPointer<ConfigWidget> widget;
    vector<QString> iconPaths;
    vector<QString> commands;
};



/** ***************************************************************************/
System::Extension::Extension()
    : Core::Extension("org.albert.extension.system"),
      Core::QueryHandler(Core::Plugin::id()),
      d(new Private) {

    registerQueryHandler(this);

    // Load settings
    for (size_t i = 0; i < NUMCOMMANDS; ++i) {
        d->iconPaths.push_back(XDG::IconLookup::iconPath(iconNames[i]));
        d->commands.push_back(settings().value(configNames[i], defaultCommand(static_cast<SupportedCommands>(i))).toString());
    }
}



/** ***************************************************************************/
System::Extension::~Extension() {

}



/** ***************************************************************************/
QWidget *System::Extension::widget(QWidget *parent) {
    if (d->widget.isNull()) {
        d->widget = new ConfigWidget(parent);

        // Initialize the content and connect the signals

        d->widget->ui.lineEdit_lock->setText(d->commands[LOCK]);
        connect(d->widget->ui.lineEdit_lock, &QLineEdit::textEdited, [this](const QString &s){
            d->commands[LOCK]= s;
            settings().setValue(configNames[LOCK], s);
        });

        d->widget->ui.lineEdit_logout->setText(d->commands[LOGOUT]);
        connect(d->widget->ui.lineEdit_logout, &QLineEdit::textEdited, [this](const QString &s){
            d->commands[LOGOUT]= s;
            settings().setValue(configNames[LOGOUT], s);
        });

        d->widget->ui.lineEdit_suspend->setText(d->commands[SUSPEND]);
        connect(d->widget->ui.lineEdit_suspend, &QLineEdit::textEdited, [this](const QString &s){
            d->commands[SUSPEND]= s;
            settings().setValue(configNames[SUSPEND], s);
        });

        d->widget->ui.lineEdit_hibernate->setText(d->commands[HIBERNATE]);
        connect(d->widget->ui.lineEdit_hibernate, &QLineEdit::textEdited, [this](const QString &s){
            d->commands[HIBERNATE]= s;
            settings().setValue(configNames[HIBERNATE], s);
        });

        d->widget->ui.lineEdit_reboot->setText(d->commands[REBOOT]);
        connect(d->widget->ui.lineEdit_reboot, &QLineEdit::textEdited, [this](const QString &s){
            d->commands[REBOOT]= s;
            settings().setValue(configNames[REBOOT], s);
        });

        d->widget->ui.lineEdit_shutdown->setText(d->commands[POWEROFF]);
        connect(d->widget->ui.lineEdit_shutdown, &QLineEdit::textEdited, [this](const QString &s){
            d->commands[POWEROFF]= s;
            settings().setValue(configNames[POWEROFF], s);
        });

         d->widget->ui.lineEdit_screenoff->setText(d->commands[SCREENOFF]);
        connect(d->widget->ui.lineEdit_screenoff, &QLineEdit::textEdited, [this](const QString &s){
            d->commands[SCREENOFF]= s;
            settings().setValue(configNames[SCREENOFF], s);
        });       
    }
    return d->widget;
}



/** ***************************************************************************/
void System::Extension::handleQuery(Core::Query * query) const {

    if ( query->string().isEmpty())
        return;

    QRegularExpression re(QString("(%1)").arg(query->string()), QRegularExpression::CaseInsensitiveOption);
    for (size_t i = 0; i < NUMCOMMANDS; ++i) {
        for (auto &alias : aliases[i]) {
            if ( alias.startsWith(query->string(), Qt::CaseInsensitive) ) {
                auto item = makeStdItem(configNames[i],
                                        d->iconPaths[i],
                                        QString(itemTitles[i]).replace(re, "<u>\\1</u>"),
                                        itemDescriptions[i],
                                        ActionList { makeProcAction(itemDescriptions[i], QStringList{"/bin/sh", "-c", d->commands[i]}) });
                query->addMatch(std::move(item), static_cast<uint>(static_cast<uint>(query->string().size() / itemTitles[i].size() * UINT_MAX)));
                break;
            }
        }
    }
}

