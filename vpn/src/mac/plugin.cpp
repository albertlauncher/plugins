// Copyright (c) 2023-2025 Manuel Schneider

#include "plugin.h"
#include "item.h"
#include <albert/logging.h>
ALBERT_LOGGING_CATEGORY("vpn")
using namespace albert;
using namespace std;


class Plugin::Private
{
public:
    vector<shared_ptr<Item>> items;
};

Plugin::Plugin() : d(make_unique<Private>())
{
    d->items = SCNetworkInterfaceItem::createItems();
}

Plugin::~Plugin() = default;

void Plugin::updateIndexItems()
{
    vector<IndexItem> items;
    for (const auto &item : d->items)
        items.emplace_back(item, item->text());
    setIndexItems(::move(items));
}
