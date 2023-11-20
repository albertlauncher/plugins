// Copyright (c) 2022 Manuel Schneider

#include "albert/albert.h"
#include "albert/extension/queryhandler/standarditem.h"
#include "plugin.h"
#include <QMetaEnum>
#include <QtDBus/QDBusMessage>
#include <QtDBus/QDBusConnection>
#include <QtDBus/QDBusInterface>
#include <QtDBus/QDBusPendingCall>
#include <QtDBus/QDBusPendingReply>
#include <QtDBus/QDBusPendingCallWatcher>

using namespace albert;
using namespace std;

/* Max timeout to wait for async dbus calls, in milisencods */
static const int DBUS_TIMEOUT = 25;

/**
 * Handle dbus async response. If there is no error, calls `onReplySuccessful` function with the reply message, if it has no errors.
 * @param watcher Async dbus call watcher pointer. Watched is disposed of after execution.
 * @param onReplySuccessful Callback that is called only if the reply has no error message.
 */
void handlePendingReply(QDBusPendingCallWatcher *watcher, std::function<void(QDBusMessage)> onReplySuccessful)
{
    QDBusPendingReply pendingReply = *watcher;
    if (pendingReply.isError())
    {
        QDBusError error = pendingReply.error();
        qWarning() << qPrintable(QString("[DBus Async] message returned error: %1").arg(error.message()));
    }
    else
    {
        QDBusMessage replyMessage = pendingReply.reply();
        if (!replyMessage.errorMessage().isEmpty())
        {
            qWarning() << qPrintable(QString("[DBus Async] message returned error: %1").arg(replyMessage.errorMessage()));
        }
        else
        {
            onReplySuccessful(replyMessage);
        }
    }
    watcher->deleteLater();
}

/**
 * Process a dbus request asynchronously.
 * @param message DBus message to send.
 * @param onReplySuccessful Callback that is called if the reply has no error message.
 */
void processMessageAsync(QDBusMessage message, std::function<void(QDBusMessage)> onReplySuccessful)
{
    QDBusConnection connection = QDBusConnection::sessionBus();
    // If there is no session bus, abort
    if (!connection.isConnected())
        return;
    QDBusPendingCall asyncCall = connection.asyncCall(message, DBUS_TIMEOUT);
    QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(asyncCall);
    if (watcher->isFinished())
    {
        handlePendingReply(watcher, onReplySuccessful);
    }
    else
    {
        // waits for the finished signal
        QObject::connect(watcher, &QDBusPendingCallWatcher::finished, [=](QDBusPendingCallWatcher *watcherReply)
                         { handlePendingReply(watcherReply, onReplySuccessful); });
    }
}

/**
 * Fetch the ids of all available dbus endpoints asynchronously.
 * @param endpointsFound Callback that is invoked with the list of endpoints, in case they're found.
 */
static void findBusEndpoints(std::function<void(QStringList)> onEndpointsFound)
{
    // Querying the DBus to list all available services
    QDBusMessage listNamesMessage = QDBusMessage::createMethodCall("org.freedesktop.DBus", "/", "org.freedesktop.DBus", "ListNames");
    processMessageAsync(listNamesMessage, [=](QDBusMessage response)
                        {
        QList<QVariant> args = response.arguments();
        if (args.length() != 1)
        {
            qWarning() << qPrintable(QString("[DBus Async]: Expected 1 argument for DBus reply. Got %1").arg(args.length()));
            return;
        }
        QVariant arg = args.at(0);
        if (arg.isNull() || !arg.isValid())
        {
            qWarning("[DBus Async]: Reply argument not valid or null!");
            return;
        }
        QStringList runningBusEndpoints = arg.toStringList();
        if (runningBusEndpoints.isEmpty())
        {
            qWarning("[DBus Async]: Argument is either not type of QStringList or is empty!");
            return;
        }
        qDebug() << qPrintable(QString("[DBus Async]: Found %1 running endpoints").arg(runningBusEndpoints.size()));
        onEndpointsFound(runningBusEndpoints); });
}

/**
 * Asks a media player with the given dbus endpoint id for a specific player property.
 *
 * @param playerId dbus endpoint id of the media player
 * @param property media player property to query. Properties available at https://specifications.freedesktop.org/mpris-spec/2.2/Player_Interface.html
 * @param onPropertyFound Callback that is invoked with the player's value for the property, in case it's found.
 */
static void findPlayerProperty(QString playerId, QString property, std::function<void(QVariant)> onPropertyFound)
{
    QDBusMessage message = QDBusMessage::createMethodCall(playerId, "/org/mpris/MediaPlayer2", "org.freedesktop.DBus.Properties", "Get");
    QList<QVariant> args;
    args.append("org.mpris.MediaPlayer2.Player");
    args.append(property);
    message.setArguments(args);
    processMessageAsync(message, [=](QDBusMessage response)
                        {
        if (response.arguments().empty())
        {
            qWarning() << "DBus: Player property query returned empty response";
        }
        else
        {
            QVariant variant = response.arguments().at(0).value<QDBusVariant>().variant();
            onPropertyFound(variant);
        } });
}

/**
 * Finds all media available media players (bus id must have a specific prefix) that have its properties matching the required values.
 *
 * @param propertyToCheck name of the `org.mpris.MediaPlayer2` property to check
 * @param expectedValue value a media player must have in order to be able to execute the command
 * @param onPlayerFound Callback that tells that the player with this ID is suitable to run the the desired command.
 */
static void findRunningMediaPlayers(QString propertyToCheck, QVariant expectedValue, std::function<void(QString)> onPlayerFound)
{
    findBusEndpoints([=](QStringList runningBusEndpoints)
                     {
        vector<QString> mediaPlayers;
        // Filter all mpris capable
        QStringList busids;
        for (QString &id : runningBusEndpoints)
        {
            if (id.startsWith("org.mpris.MediaPlayer2."))
            {
                qDebug() << qPrintable(QString("[DBus Async] Found running media player %1").arg(id));
                findPlayerProperty(id, propertyToCheck, [=](QVariant playerProperty)
                                   {
                    qDebug() << qPrintable(QString("[DBus Async] Player has property %1 = %2").arg(propertyToCheck, playerProperty.toString()));
                    if (playerProperty == expectedValue)
                    {
                        onPlayerFound(id);
                    } else {
                        qWarning() << qPrintable(QString("[DBus Async] Player %1 does not match the expected value %2 (was %3)").arg(id, expectedValue.toString(), playerProperty.toString()));
                    } });
            }
        } });
}

/**
 * Runs a command in all media players whose property matches the expected value.
 *
 * @param command command to run on matching player. See this for a list of commands: https://specifications.freedesktop.org/mpris-spec/2.2/Player_Interface.html
 * @param property name of the player's property to evaluate
 * @param expectedValue value the property must have in order for the player to be able to run the command
 */
static void runMediaCommand(QString command, QString property, QVariant expectedValue)
{
    findRunningMediaPlayers(property, expectedValue, [=](QString mediaPlayer)
                            {
        qDebug() << qPrintable(QString("[DBus Async] Calling %1 on player %2").arg(command, mediaPlayer));
        QDBusMessage msg = QDBusMessage::createMethodCall(mediaPlayer, "/org/mpris/MediaPlayer2", "org.mpris.MediaPlayer2.Player", command);
        processMessageAsync(msg, [=](QDBusMessage response) { 
            (void) response;
            qDebug() << qPrintable(QString("[DBus Async] Calling command %1 was successful").arg(command)); 
            }); });
}

/**
 * Creates an MPRIS action item with the given attributes.
 *
 * @param title title of the action.
 * @param description description of the action.
 * @param icon_urls list of icon urls to use for the action.
 * @param command command to run on matching player. See this for a list of commands: https://specifications.freedesktop.org/mpris-spec/2.2/Player_Interface.html
 * @param property name of the player's property to evaluate
 * @param expectedValue value the property must have in order for the player to be able to run the command
 * @return a pointer to the created item.
 */
static shared_ptr<Item> createItem(QString title, QString description, QStringList icon_urls, QString command, QString property, QVariant expectedValue)
{
    vector<Action> actions = {{title, description,
                               [=]()
                               { runMediaCommand(command, property, expectedValue); }}};
    return StandardItem::make(
        title,
        title,
        description,
        icon_urls,
        actions);
};

/**
 * An MPRIS action item that pauses all media players that can be paused.
 * @return a pointer to the created item.
 */
static shared_ptr<Item> pauseItem()
{
    return createItem("Pause", "Pause all playing media", QStringList({"qsp:SP_MediaPause"}), "Pause", "CanPause", QVariant(true));
}

/**
 * An MPRIS action item that plays all media players that can be played.
 * @return a pointer to the created item.
 */
static shared_ptr<Item> playItem()
{
    return createItem("Play", "Play all paused media", QStringList({"qsp:SP_MediaPlay"}), "Play", "CanPlay", QVariant(true));
}

/**
 * An MPRIS action item that stops all media players that can be paused.
 * @return a pointer to the created item.
 */
static shared_ptr<Item> stopItem()
{
    return createItem("Stop", "Stop all playing media", QStringList({"qsp:SP_MediaStop"}), "Stop", "CanPause", QVariant(true));
}

/**
 * An MPRIS action item that skips forward on all media players that can go next.
 * @return a pointer to the created item.
 */
static shared_ptr<Item> nextItem()
{
    return createItem("Next", "Skip forward on playing media", QStringList({"qsp:SP_MediaSkipForward"}), "Next", "CanGoNext", QVariant(true));
}

/**
 * An MPRIS action item that skips backward on all media players that can go previous.
 * @return a pointer to the created item.
 */
static shared_ptr<Item> previousItem()
{
    return createItem("Previous", "Skip backward on playing media", QStringList({"qsp:SP_MediaSkipBackward"}), "Previous", "CanGoPrevious", QVariant(true));
}

/**
 * Creates a list of MPRIS action items that match the given query.
 * @param query query string to match.
 * @return a list of pointers to the created items.
 */
vector<shared_ptr<Item>> itemsForQuery(QString query)
{
    vector<shared_ptr<Item>> items;
    if (query.size() >= 1)
    {
        QString queryLower = query.toLower();
        if (QString("pause").startsWith(queryLower))
        {
            items.emplace_back(pauseItem());
        }
        if (QString("play").startsWith(queryLower))
        {
            items.emplace_back(playItem());
        }
        if (QString("stop").startsWith(queryLower))
        {
            items.emplace_back(stopItem());
        }
        if (QString("next").startsWith(queryLower))
        {
            items.emplace_back(nextItem());
        }
        if (QString("previous").startsWith(queryLower))
        {
            items.emplace_back(previousItem());
        }
    }
    return items;
}

/**
 * Handler for the global query, without any prefix.
 * Called by the albert core.
 */
vector<RankItem> Plugin::handleGlobalQuery(const GlobalQuery *query) const
{
    vector<shared_ptr<Item>> items = itemsForQuery(query->string());
    vector<RankItem> results;
    for (shared_ptr<Item> item : items)
        results.emplace_back(item, 1.0f);
    return results;
}

/**
 * Handler for the queries that start with the `mpris` trigger.
 * Called by the albert core.
 */
void Plugin::handleTriggerQuery(TriggerQuery *query) const
{
    vector<shared_ptr<Item>> items = itemsForQuery(query->string());
    for (shared_ptr<Item> item : items)
        query->add(item);
}
