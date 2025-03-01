// Copyright (c) 2023-2025 Manuel Schneider

#pragma once
#include "vpnconnectionitem.h"

class SCNetworkInterfaceItem : public VpnConnectionItem
{
public:

    SCNetworkInterfaceItem(const QString &service_id, const QString &service_name);
    ~SCNetworkInterfaceItem();

    QString id() const override;
    QString text() const override;
    std::vector<albert::Action> actions() const override;

    static std::vector<std::shared_ptr<Item>> createItems();

private:

    void setConnected(bool connect) const;
    void updateState();

    class Private;
    std::unique_ptr<Private> d;
};
