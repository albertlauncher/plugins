// Copyright (c) 2023-2025 Manuel Schneider

#include "plugin.h"
#include "networkmanager.h"
#include <albert/logging.h>
ALBERT_LOGGING_CATEGORY("vpn")
using namespace albert;
using namespace std;

class Plugin::Private
{
public:
    NetworkManager nm;
};

Plugin::Plugin()
{
    if (!QDBusConnection::systemBus().isConnected())
        throw runtime_error("Failed to connect to the system bus.");

    qRegisterMetaType<NestedVariantMap>("NestedVariantMap");
    qDBusRegisterMetaType<NestedVariantMap>();
    qDBusRegisterMetaType<QVariantList>();

    // Defer nm creation
    d = make_unique<Private>();
}

Plugin::~Plugin() = default;

void Plugin::updateIndexItems()
{
    vector<IndexItem> items;
    for (const auto &item : d->nm.items())
        items.emplace_back(item, item->text());
    setIndexItems(::move(items));
}
