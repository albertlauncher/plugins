// Copyright (c) 2023-2025 Manuel Schneider

#pragma once
#include <albert/item.h>
#include "plugin.h"
#include "vpnconnectionitem.h"
#include <QDBusObjectPath>

class NetworkManager;
class OrgFreedesktopNetworkManagerConnectionActiveInterface;
using IActiveConnection = OrgFreedesktopNetworkManagerConnectionActiveInterface;

class VpnItem :  public VpnConnectionItem
{
public:
    VpnItem(NetworkManager &manager, const QString &n, const QDBusObjectPath &p);

    QString id() const override;
    QString text() const override;
    std::vector<albert::Action> actions() const override;

private:

    NetworkManager &nm;
    const QString name;
    const QDBusObjectPath object_path_settings;
    std::unique_ptr<IActiveConnection> active_connection;

    friend class NetworkManager;

};

