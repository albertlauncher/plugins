// Copyright (c) 2023-2025 Manuel Schneider

#include "networkmanager.h"
#include "vpnconnectionitem.h"
#include <albert/logging.h>
using namespace albert;
using namespace std;
static constexpr const char *service = "org.freedesktop.NetworkManager";
static constexpr const char *object_path_manager  = "/org/freedesktop/NetworkManager";
static constexpr const char *object_path_settings = "/org/freedesktop/NetworkManager/Settings";


template<typename T>
static T getNestedVariantMapValue(const NestedVariantMap &map, const QString &k1, const QString &k2)
{
    auto it_inner_map = map.find(k1);
    if (it_inner_map == map.end())
        throw runtime_error("");

    auto it_value = it_inner_map->find(k2);
    if (it_value == it_inner_map->end())
        throw runtime_error("");

    if (!it_value->canConvert<T>())
        throw runtime_error("");

    return it_value->value<T>();
}

// https://people.freedesktop.org/~lkundrak/nm-docs/nm-dbus-types.html#NMActiveConnectionState
static VpnConnectionItem::State toState(int status)
{
    // NM_ACTIVE_CONNECTION_STATE_UNKNOWN = 0
    // NM_ACTIVE_CONNECTION_STATE_ACTIVATING = 1
    // NM_ACTIVE_CONNECTION_STATE_ACTIVATED = 2
    // NM_ACTIVE_CONNECTION_STATE_DEACTIVATIN = 3
    // NM_ACTIVE_CONNECTION_STATE_DEACTIVATED = 4

    switch (status) {
    case 0:
        return VpnConnectionItem::State::Invalid;
    case 1:
        return VpnConnectionItem::State::Connecting;
    case 2:
        return VpnConnectionItem::State::Connected;
    case 3:
        return VpnConnectionItem::State::Disconnecting;
    case 4:
        return VpnConnectionItem::State::Disconnected;
    }
    return VpnConnectionItem::State::Invalid;
}


NetworkManager::NetworkManager():
    manager(service, object_path_manager, QDBusConnection::systemBus()),
    properties(service, object_path_manager, QDBusConnection::systemBus())
{
    connect(&properties, &IProperties::PropertiesChanged,
            this, &NetworkManager::onPropertiesChanged);

    ISettings settings(service, object_path_settings, QDBusConnection::systemBus());

    auto reply = settings.ListConnections();
    reply.waitForFinished();

    for (auto object_path : reply.value())
    {
        auto connection = IConnection(service, object_path.path(), QDBusConnection::systemBus());

        auto r = connection.GetSettings();
        r.waitForFinished();

        auto conn_settings = r.value();

                // for (const auto&[k,v] : connection_settings.asKeyValueRange())
                //     CRIT << k << v;

        try
        {
            auto name = getNestedVariantMapValue<QString>(conn_settings, "connection", "id");
            auto type = getNestedVariantMapValue<QString>(conn_settings, "connection", "type");

            if (type == "wireguard" || type == "vpn")
                items_.emplace_back(make_shared<VpnItem>(*this, name, object_path));
        }
        catch (...)
        {
            continue;
        }
    }

    // initialize states
    handleActiveConnectionsChanged(manager.activeConnections());
}

void NetworkManager::onPropertiesChanged(const QString &interface,
                                         const QVariantMap &changed,
                                         const QStringList &)
{
    if (interface == IManager::staticInterfaceName())
        if (auto it = changed.find("ActiveConnections"); it != changed.end())
            handleActiveConnectionsChanged(qdbus_cast<QList<QDBusObjectPath>>(*it));
}

void NetworkManager::handleActiveConnectionsChanged(
    const QList<QDBusObjectPath> &active_connection_paths)
{
    vector<unique_ptr<IActiveConnection>> active_connections;
    for (const auto &p : active_connection_paths)
        active_connections.emplace_back(
            make_unique<IActiveConnection>(service, p.path(), QDBusConnection::systemBus()));



    for (auto &item : items_)
        if (auto it = ranges::find_if(active_connections,
                                      [&](const auto &c) {
                                          return c->connection() == item->object_path_settings;
                                      });
            it == active_connections.end())
        {
            item->active_connection.reset();
            item->setState(VpnConnectionItem::State::Disconnected);
        }
        else
        {
            if (!item->active_connection)
            {
                item->active_connection = ::move(*it);
                connect(item->active_connection.get(), &IActiveConnection::StateChanged,
                        this, [&item](uint state, uint /*reason*/){ item->setState(toState(state)); });
                item->setState(toState(item->active_connection->state()));
            }
        }
}

void NetworkManager::activate(const VpnItem &item)
{
    manager.ActivateConnection(item.object_path_settings,
                               QDBusObjectPath("/"),  // ignored
                               QDBusObjectPath("/"));  // auto choice
}

void NetworkManager::deactivate(const VpnItem &item)
{
    if (item.active_connection)
        manager.DeactivateConnection(item.active_connection->connection());
}

const vector<shared_ptr<VpnItem> > &NetworkManager::items() { return items_; }
