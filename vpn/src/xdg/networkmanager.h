// Copyright (c) 2023-2025 Manuel Schneider

#pragma once
#include "item.h"
#include "nm.h"

class NetworkManager : public QObject
{
    // Q_OBJECT

public:

    NetworkManager();

    void activate(const VpnItem &item);
    void deactivate(const VpnItem &item);

    const std::vector<std::shared_ptr<VpnItem>> &items();

private:

    void onPropertiesChanged(const QString &interface, const QVariantMap &changed, const QStringList &/*invalidated*/);
    void handleActiveConnectionsChanged(const QList<QDBusObjectPath> &active_connection_paths);

    IManager manager;
    IProperties properties;
    std::vector<std::shared_ptr<VpnItem>> items_;
};

