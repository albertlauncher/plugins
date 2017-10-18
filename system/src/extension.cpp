// Copyright (C) 2014-2017 Manuel Schneider

#include <QDebug>
#include <QPointer>
#include <QProcess>
#include <QSettings>
#include <vector>
#include "configwidget.h"
#include "extension.h"
#include "util/standardactions.h"
#include "util/standarditem.h"
#include "xdg/iconlookup.h"
using namespace std;
using namespace Core;


namespace {

enum SupportedCommands {
    LOCK,
    LOGOUT,
    SUSPEND,
    HIBERNATE,
    REBOOT,
    POWEROFF,
    NUMCOMMANDS
};

vector<QString> configNames = {
    "lock",
    "logout",
    "suspend",
    "hibernate",
    "reboot",
    "shutdown"
};

vector<QString> itemTitles = {
    "Lock",
    "Log out",
    "Suspend",
    "Hibernate",
    "Restart",
    "Shut down"
};

vector<QString> itemDescriptions = {
    "Lock the session.",
    "Quit the session.",
    "Suspend the machine.",
    "Hibernate the machine.",
    "Reboot the machine.",
    "Shutdown the machine.",
};

vector<QString> iconNames = {
    "system-lock-screen",
    "system-log-out",
    "system-suspend",
    "system-suspend-hibernate",
    "system-reboot",
    "system-shutdown"
};


QString defaultCommand(SupportedCommands command){

    QString de = getenv("XDG_CURRENT_DESKTOP");

    switch (command) {
    case LOCK:
        if (de == "Unity" || de == "Pantheon" || de == "GNOME")
            return "gnome-screensaver-command --lock";
        else if (de == "kde-plasma" || de == "KDE")
            return "dbus-send --dest=org.freedesktop.ScreenSaver --type=method_call /ScreenSaver org.freedesktop.ScreenSaver.Lock";
        else if (de == "XFCE")
            return "xflock4";
        else if (de == "X-Cinnamon" || de == "Cinnamon")
            return "cinnamon-screensaver-command --lock";
        else if (de == "MATE")
            return "mate-screensaver-command --lock";
        else
            return "xdg-screensaver lock";

    case LOGOUT:
        if (de == "Unity" || de == "Pantheon" || de == "GNOME")
            return "gnome-session-quit --logout";
        else if (de == "kde-plasma" || de == "KDE")
            return "qdbus org.kde.ksmserver /KSMServer logout 0 0 0";
        else if (de == "X-Cinnamon")
            return "cinnamon-session-quit --logout";
        else if (de == "XFCE")
            return "xfce4-session-logout --logout";
        else if (de == "MATE")
            return "mate-session-save --logout";
        else
            return "notify-send \"Error.\" \"Logout command is not set.\" --icon=system-log-out";

    case SUSPEND:
        if (de == "XFCE")
            return "xfce4-session-logout --suspend";
        else if (de == "MATE")
            return "sh -c \"mate-screensaver-command --lock && systemctl suspend -i\"";
        else
            return "systemctl suspend -i";

    case HIBERNATE:
        if (de == "XFCE")
            return "xfce4-session-logout --hibernate";
        else if (de == "MATE")
            return "sh -c \"mate-screensaver-command --lock && systemctl hibernate -i\"";
        else
            return "systemctl hibernate -i";

    case REBOOT:
        if (de == "Unity" || de == "Pantheon" || de == "GNOME")
            return "gnome-session-quit --reboot";
        else if (de == "kde-plasma" || de == "KDE")
            return "qdbus org.kde.ksmserver /KSMServer logout 0 1 0";
        else if (de == "X-Cinnamon")
            return "cinnamon-session-quit --reboot";
        else if (de == "XFCE")
            return "xfce4-session-logout --reboot";
        else if (de == "MATE")
            return "mate-session-save --shutdown-dialog";
        else
            return "notify-send \"Error.\" \"Reboot command is not set.\" --icon=system-reboot";

    case POWEROFF:
        if (de == "Unity" || de == "Pantheon" || de == "GNOME")
            return "gnome-session-quit --power-off";
        else if (de == "kde-plasma" || de == "KDE")
            return "qdbus org.kde.ksmserver /KSMServer logout 0 2 0";
        else if (de == "X-Cinnamon")
            return "cinnamon-session-quit --power-off";
        else if (de == "XFCE")
            return "xfce4-session-logout --halt";
        else if (de == "MATE")
            return "mate-session-save --shutdown-dialog";
        else
            return "notify-send \"Error.\" \"Poweroff command is not set.\" --icon=system-shutdown";

    case NUMCOMMANDS:
        // NEVER REACHED;
        return "";
    }

    // NEVER REACHED;
    return "";
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
    }
    return d->widget;
}



/** ***************************************************************************/
void System::Extension::handleQuery(Core::Query * query) const {

    if ( query->string().isEmpty())
        return;

    for (size_t i = 0; i < NUMCOMMANDS; ++i) {
        if ( itemTitles[i].startsWith(query->string(), Qt::CaseInsensitive) ) {
            auto item = std::make_shared<Core::StandardItem>(configNames[i]);
            item->setText(itemTitles[i]);
            item->setSubtext(itemDescriptions[i]);
            item->setIconPath(d->iconPaths[i]);
            item->addAction(make_shared<ProcAction>(itemDescriptions[i], QStringList(d->commands[i])));
            query->addMatch(std::move(item), static_cast<uint>(static_cast<float>(query->string().size())/itemTitles[i].size()*UINT_MAX));
        }
    }
}

