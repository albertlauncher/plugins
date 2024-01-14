// Copyright (c) 2017-2024 Manuel Schneider

#include "albert/albert.h"
#include "albert/extension/queryhandler/standarditem.h"
#include "albert/logging.h"
#include "plugin.h"
#include "ui_configwidget.h"
#include <QDBusConnection>
#include <QDBusConnectionInterface>
#include <QDBusInterface>
#include <QDBusServiceWatcher>
#include <QWidget>
#include <QXmlStreamReader>
ALBERT_LOGGING_CATEGORY("mpris")
using namespace albert;
using namespace std;

static const int dbus_timeout = 100;
static const char * dbus_service_name = "org.mpris.MediaPlayer2";
static const char * dbus_object_path = "/org/mpris/MediaPlayer2";
static const char * dbus_iface_player = dbus_service_name;
static const char * dbus_iface_control = "org.mpris.MediaPlayer2.Player";
static const char * dbus_iface_introspecable = "org.freedesktop.DBus.Introspectable";
static const char * dbus_iface_properties = "org.freedesktop.DBus.Properties";

struct ThrowingQDBusInterface : private QDBusInterface
{
    ThrowingQDBusInterface(const QString &service, const QString &path, const QString &iface)
        : QDBusInterface(service, path, iface)
    {
        if (!isValid())
            throw runtime_error(lastError().message().toStdString());
        setTimeout(dbus_timeout);
    }

    template<class TYPE>
    TYPE getProperty(const char * property_name) const
    {
        if (auto var = property(property_name); lastError().isValid())
            throw runtime_error(lastError().message().toStdString());
        else
            return var.value<TYPE>();
    }

    template<class TYPE>
    TYPE call(const QString &method_name) {
        QDBusReply<TYPE> reply = QDBusAbstractInterface::call(QDBus::Block, method_name);
        if (reply.isValid())
            return reply.value();
        else
            throw runtime_error(reply.error().message().toStdString());
    }
};

// specialize void
template <>
void ThrowingQDBusInterface::call(const QString &method_name) {
    QDBusReply<void> reply = QDBusAbstractInterface::call(QDBus::Block, method_name);
    if (!reply.isValid())
        throw runtime_error(reply.error().message().toStdString());
}

// https://specifications.freedesktop.org/mpris-spec/2.2/Media_Player.html
// https://specifications.freedesktop.org/mpris-spec/2.2/Player_Interface.html
class Player
{
    const QString dbus_service_name;
    ThrowingQDBusInterface player;
    ThrowingQDBusInterface control;
    QString identity;

public:
    Player(const QString &service_name):
        dbus_service_name(service_name),
        player(dbus_service_name, dbus_object_path, dbus_iface_player),
        control(dbus_service_name, dbus_object_path, dbus_iface_control)
    {
        bool have_player = false, have_control = false, have_properties = false;
        ThrowingQDBusInterface introspectable(dbus_service_name, dbus_object_path, dbus_iface_introspecable);
        QXmlStreamReader xml(introspectable.call<QString>("Introspect"));

        while (!xml.atEnd() && !xml.hasError())
            if (auto token = xml.readNext(); token == QXmlStreamReader::StartElement)
                if (xml.name() == QStringLiteral("interface"))
                    if(auto interface_name = xml.attributes().value(QStringLiteral("name")).toString();
                        !interface_name.isEmpty()){
                        if (interface_name == dbus_iface_player)
                            have_player = true;
                        else if (interface_name == dbus_iface_control)
                            have_control = true;
                        else if (interface_name == dbus_iface_properties)
                            have_properties = true;
                    }

        if (xml.hasError())
            throw runtime_error(xml.errorString().toStdString());

        if (!(have_player && have_control && have_properties))
            throw runtime_error("Service does not provide all required interfaces.");

        if (!control.getProperty<bool>("CanControl"))
            throw runtime_error("This player does not allow control.");

        identity = player.getProperty<QString>("Identity");
    }

    shared_ptr<Item> buildControlItem(const QString &command, const QStringList &icon_urls)
    {
        static const auto tr = QCoreApplication::translate("Player", "%1 media player control");
        return StandardItem::make(
            identity + command,
            command,
            tr.arg(identity),
            icon_urls,
            {{ command, command, [this, command]() { control.call<void>(command); } }}
        );
    }

    vector<shared_ptr<Item>> createItems(const QString &query)
    {
        vector<shared_ptr<Item>> items;

        // One of:
        // Playing (Playing) - A track is currently playing.
        // Paused (Paused) - A track is currently paused.
        // Stopped (Stopped) - There is no track currently playing.
        auto playbackStatus = control.getProperty<QString>("PlaybackStatus");
        bool playing = (playbackStatus == QStringLiteral("Playing"));

        if (playing)
        {
            if (static const auto cmd = QCoreApplication::translate("Player", "Stop");
                cmd.startsWith(query, Qt::CaseInsensitive))
                items.push_back(buildControlItem(cmd, {"xdg:media-playback-stop", "qsp:SP_MediaStop"}));


            if (static const auto cmd = QCoreApplication::translate("Player", "Pause");
                cmd.startsWith(query, Qt::CaseInsensitive) && control.getProperty<bool>("CanPause"))
                items.push_back(buildControlItem(cmd, {"xdg:media-playback-pause", "qsp:SP_MediaPause"}));
        }

        else if (static const auto cmd = QCoreApplication::translate("Player", "Play");
                 cmd.startsWith(query, Qt::CaseInsensitive) && control.getProperty<bool>("CanPlay"))
                items.push_back(buildControlItem(cmd, {"xdg:media-playback-start", "qsp:SP_MediaPlay"}));

        if (static const auto cmd = QCoreApplication::translate("Player", "Next");
            cmd.startsWith(query, Qt::CaseInsensitive) && control.getProperty<bool>("CanGoNext"))
            items.push_back(buildControlItem(cmd, {"xdg:media-skip-forward", "qsp:SP_MediaSkipForward"}));

        if (static const auto cmd = QCoreApplication::translate("Player", "Previous");
            cmd.startsWith(query, Qt::CaseInsensitive) && control.getProperty<bool>("CanGoPrevious"))
            items.push_back(buildControlItem(cmd, {"xdg:media-skip-backward", "qsp:SP_MediaSkipBackward"}));


        // TODO: add a player item
        //    getProperty<bool>("CanRaise");
        //    call<void>(QStringLiteral("Raise"));
        //    getProperty<bool>("CanQuit");
        //    call<void>(QStringLiteral("Quit"));
        //    getProperty<QString>("LoopStatus");
        //    QVariantMap metadata() {
        ////        a{sv} (Metadata_Map)	Read only
        //        /*
        //         * Variant: [Argument: a{sv}
        //         *  {
        //         *      "mpris:trackid" = [Variant(QString): "/com/spotify/track/6G6SAy7BwNGKttAFGjADmQ"],
        //         *      "mpris:length" = [Variant(qulonglong): 396960000],
        //         *      "mpris:artUrl" = [Variant(QString): "https://i.scdn.co/image/ab67616d0000b27332466645f8f89b23a59a6af3"],
        //         *      "xesam:album" = [Variant(QString): "Ciclos"],
        //         *      "xesam:albumArtist" = [Variant(QStringList): {"Antaares"}],
        //         *      "xesam:artist" = [Variant(QStringList): {"Antaares"}],
        //         *      "xesam:autoRating" = [Variant(double): 0.39],
        //         *      "xesam:discNumber" = [Variant(int): 1],
        //         *      "xesam:title" = [Variant(QString): "Ciclos"],
        //         *      "xesam:trackNumber" = [Variant(int): 1],
        //         *      "xesam:url" = [Variant(QString): "https://open.spotify.com/track/6G6SAy7BwNGKttAFGjADmQ"]
        //         *  }
        //         */
        //        return {};
        //    }
        //};

        return items;
    }
};

struct Plugin::Private
{
    std::map<QString, Player> players;
    QDBusServiceWatcher service_watcher;

    void tryAddPlayer(const QString &service)
    {
        try {
            players.emplace(service, service);
            DEBG << "MPRIS player registered:" << service;
        } catch (const runtime_error &e) {
            WARN << "Error while creating player" << service << e.what();
        }
    }
};

Plugin::Plugin() : d(new Private)
{
    if (!QDBusConnection::sessionBus().isConnected())
        throw runtime_error("Failed to connect to session bus.");

    // Each media player must request a unique bus name which begins with org.mpris.MediaPlayer2
    if (auto reply = QDBusConnection::sessionBus().interface()->registeredServiceNames(); !reply.isValid())
        throw runtime_error(reply.error().message().toStdString());
    else
        for (const auto &service : reply.value())
            if (service.startsWith(QStringLiteral("org.mpris.MediaPlayer2.")))
                d->tryAddPlayer(service);

    // Track new players
    d->service_watcher.setConnection(QDBusConnection::sessionBus());
    d->service_watcher.setWatchMode(QDBusServiceWatcher::WatchForOwnerChange);
    d->service_watcher.addWatchedService(QStringLiteral("org.mpris.MediaPlayer2*"));
    connect(&d->service_watcher, &QDBusServiceWatcher::serviceOwnerChanged, this,
            [this](const QString &service, const QString&, const QString &newOwner){
        if(d->players.erase(service))
            DEBG << "MPRIS player unregistered:" << service;
        if (!newOwner.isEmpty())
            d->tryAddPlayer(service);
    });
}

Plugin::~Plugin() = default;

vector<RankItem> Plugin::handleGlobalQuery(const GlobalQuery *query) const
{
    vector<RankItem> results;
    for (auto &[service, player] : d->players)
        try {
            for (const auto& item : player.createItems(query->string()))
                results.emplace_back(item, (double)query->string().size()/item->text().size());
        } catch (const runtime_error &e) {
            WARN << e.what();
        }
    return results;
}


QWidget *Plugin::buildConfigWidget()
{
    auto *w = new QWidget();
    Ui::ConfigWidget ui;
    ui.setupUi(w);
    return w;
}
