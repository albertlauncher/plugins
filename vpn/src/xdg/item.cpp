// Copyright (c) 2023-2025 Manuel Schneider

#include "item.h"
#include "plugin.h"
#include "networkmanager.h"
using namespace albert;
using namespace std;

VpnItem::VpnItem(NetworkManager &manager, const QString &n, const QDBusObjectPath &p) :
    nm(manager),
    name(n),
    object_path_settings(p)
{}

QString VpnItem::id() const { return object_path_settings.path(); }

QString VpnItem::text() const { return name; }

vector<Action> VpnItem::actions() const
{
    if (state() == State::Connected && active_connection)
        return {Action("deact", Plugin::tr("Disconnect"),
                       [this] { nm.deactivate(*this); })};

    else if (state() == State::Disconnected)
        return {Action("act", Plugin::tr("Connect"),
                       [this] { nm.activate(*this); })};

    else
        return {};
}
