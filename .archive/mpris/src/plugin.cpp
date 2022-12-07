// Copyright (c) 2022 Manuel Schneider

#include <QDBusConnection>
#include <QDBusInterface>
#include <QDBusMessage>
#include <QLabel>
#include <QMap>
#include <QPointer>
#include <QStringList>
#include "albert.h"
#include "command.h"
#include "plugin.h"
#include "player.h"
LOGGING_CATEGORY("mpris")

namespace  {
static const int DBUS_TIMEOUT = 25 /* ms */;
}

struct Plugin::Private
{
    ~Private()
    {
        // If there are still media player objects, delete them
        qDeleteAll(mediaPlayers);
        // Don't need to destruct the command objects.
        // This is done by the destructor of QMap
    }

    QList<Player *> mediaPlayers;
    QStringList commands;
    QMap<QString, Command> commandObjects;

};



Plugin::Plugin() : d(new Private)
{
    // Setup the DBus commands
    Command* nextToAdd = new Command(
                "play", // Label
                "Play", // Title
                "Start playing on %1", // Subtext
                "Play", // DBus Method
                {"xdg://media-playback-start", "qrc://play"}
                );
    nextToAdd->applicableWhen("/org/mpris/MediaPlayer2", "org.mpris.MediaPlayer2.Player.PlaybackStatus", "Playing", false);
    d->commands.append("play");
    d->commandObjects.insert("play", *nextToAdd);

    nextToAdd = new Command(
                "pause",
                "Pause",
                "Pause %1",
                "Pause",
                {"xdg://media-playback-pause", "qrc://pause"}
                );
    nextToAdd->applicableWhen("/org/mpris/MediaPlayer2", "org.mpris.MediaPlayer2.Player.PlaybackStatus", "Playing", true);
    d->commands.append("pause");
    d->commandObjects.insert("pause", *nextToAdd);

    nextToAdd = new Command(
                "stop",
                "Stop",
                "Stop %1",
                "Stop",
                {"xdg://media-playback-stop", "qrc://stop"}
                );
    nextToAdd->applicableWhen("/org/mpris/MediaPlayer2", "org.mpris.MediaPlayer2.Player.PlaybackStatus", "Playing", true);
    d->commands.append("stop");
    d->commandObjects.insert("stop", *nextToAdd);

    nextToAdd = new Command(
                "next track",
                "Next track",
                "Play next track on %1",
                "Next",
                {"xdg://media-skip-forward", "qrc://next"}
                );
    nextToAdd->applicableWhen("/org/mpris/MediaPlayer2", "org.mpris.MediaPlayer2.Player.CanGoNext", true, true);
    //.fireCallback([](){qInfo("NEXT");})
    d->commands.append("next track");
    d->commandObjects.insert("next track", *nextToAdd);

    nextToAdd = new Command(
                "previous track",
                "Previous track",
                "Play previous track on %1",
                "Previous",
                {"xdg://media-skip-backward", "qrc://prev"}
                );
    nextToAdd->applicableWhen("/org/mpris/MediaPlayer2", "org.mpris.MediaPlayer2.Player.CanGoPrevious", true, true);
    d->commands.append("previous track");
    d->commandObjects.insert("previous track", *nextToAdd);
}


Plugin::~Plugin()
{

}


void Plugin::setupSession()
{
    // Clean the memory
    qDeleteAll(d->mediaPlayers);
    d->mediaPlayers.clear();

    // If there is no session bus, abort
    if (!QDBusConnection::sessionBus().isConnected())
        return;

    // Querying the DBus to list all available services

    auto && method_call = QDBusMessage::createMethodCall("org.freedesktop.DBus", "/", "org.freedesktop.DBus", "ListNames");
    QDBusMessage response = QDBusConnection::sessionBus().call(method_call, QDBus::Block, DBUS_TIMEOUT);

    // Do some error checking
    if (response.type() == QDBusMessage::ReplyMessage) {
        QList<QVariant> args = response.arguments();
        if (args.length() == 1) {
            QVariant arg = args.at(0);
            if (!arg.isNull() && arg.isValid()) {
                QStringList runningBusEndpoints = arg.toStringList();
                if (!runningBusEndpoints.isEmpty()) {
                    // No errors

                    // Filter all mpris capable
                    //names = names.filter(filterRegex);
                    QStringList busids;
                    for (QString& id: runningBusEndpoints) {
                        if (id.startsWith("org.mpris.MediaPlayer2."))
                            busids.append(id);
                    }

                    for (QString& busId : busids) {

                        // QueryExecution the name of the media player of which we have the bus id.
                        QDBusInterface iface(busId, "/org/mpris/MediaPlayer2", "org.mpris.MediaPlayer2");
                        iface.setTimeout(DBUS_TIMEOUT);

                        QString name = busId;
                        QVariant prop = iface.property("Identity");
                        if (prop.isValid() && !prop.isNull() && prop.canConvert(QVariant::String)) {
                            name = prop.toString();
                        } else {
                            qWarning("DBus: Name is either invalid, null or not instanceof string");
                        }

                        bool canRaise = false;
                        prop = iface.property("CanRaise");
                        if (prop.isValid() && !prop.isNull() && prop.canConvert(QVariant::Bool)) {
                            canRaise = prop.toBool();
                        } else {
                            qWarning("DBus: CanRaise is either invalid, null or not instanceof bool");
                        }

                        // And add their player object to the list
                        d->mediaPlayers.push_back(new Player{busId, name, canRaise});

                    }


                } else
                    CRIT << "DBus error: Argument is either not type of QStringList or is empty!";
            } else
                CRIT << "DBus error: Reply argument not valid or null!";
        } else
            CRIT << "DBus error: Expected 1 argument for DBus reply. Got" << args.length();
    } else
        CRIT << "DBus error:" << response.errorMessage();
}


void Plugin::handleQuery(albert::Query *query) const
{
    const QString& q = query->string().trimmed().toLower();

    if ( q.isEmpty() )
        return;

    // Do not proceed if there are no players running. Why would you even?
    if (d->mediaPlayers.isEmpty())
        return;

    // Filter applicable commands
    QStringList cmds;
    for (QString& cmd : d->commands) {
        if (cmd.startsWith(q))
            cmds.append(cmd);
    }


    // For every option create entries for every player
    for (QString& cmd: cmds) {
        // Get the command
        Command& toExec = d->commandObjects.find(cmd).value();
        // For every player:
        for (Player *p : d->mediaPlayers) {
            // See if it's applicable for this player
            if (toExec.isApplicable(*p))
                // And add a match if so
                query->addMatch(toExec.produceAlbertItem(*p),
                                static_cast<uint>(1.0*q.length()/cmd.length())*UINT_MAX);
        }
    }
}
