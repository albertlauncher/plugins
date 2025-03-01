// Copyright (c) 2023-2025 Manuel Schneider

#pragma once
#include <QVariantMap>

using NestedVariantMap = QMap<QString, QVariantMap>;
Q_DECLARE_METATYPE(NestedVariantMap)

class OrgFreedesktopDBusPropertiesInterface;
class OrgFreedesktopNetworkManagerInterface;
class OrgFreedesktopNetworkManagerSettingsInterface;
class OrgFreedesktopNetworkManagerSettingsConnectionInterface;
class OrgFreedesktopNetworkManagerConnectionActiveInterface;

using IProperties = OrgFreedesktopDBusPropertiesInterface;
using IManager    = OrgFreedesktopNetworkManagerInterface;
using ISettings   = OrgFreedesktopNetworkManagerSettingsInterface;
using IConnection = OrgFreedesktopNetworkManagerSettingsConnectionInterface;
using IActiveConnection = OrgFreedesktopNetworkManagerConnectionActiveInterface;

