// Copyright (c) 2023-2024 Manuel Schneider

#include "plugin.h"
#include <QApplication>
#include <QMessageBox>
#include <albert/albert.h>
#include <albert/logging.h>
#include "nm.h"
ALBERT_LOGGING_CATEGORY("vpn")
using namespace albert;
using namespace std;
using IManager = OrgFreedesktopNetworkManagerInterface;
using ISettings = OrgFreedesktopNetworkManagerSettingsInterface;
using IConnectionSettings = OrgFreedesktopNetworkManagerSettingsConnectionInterface;


static const char *service = "org.freedesktop.NetworkManager";
static const char *manager_path = "/org/freedesktop/NetworkManager";
static const char *settings_path = "/org/freedesktop/NetworkManager/Settings";



class VpnItem : public Item
{
    QString uuid;
    QString name;
    QDBusObjectPath object_path;

public:

    VpnItem(const QString &i, const QString &n, const QDBusObjectPath &p):
        uuid(i), name(n), object_path(p)
    {

    }

    QString id() const override { return uuid; }

    QString text() const override { return name; }

    QString subtext() const override { return object_path.path(); }

    QStringList iconUrls() const override
    {
        //switch (status()) {
        //default:
            return {QStringLiteral("gen:?&text=🔓")};
        //}
    }

    QString inputActionText() const override { return text(); }

    vector<Action> actions() const override
    {
        if (isActive())
            return {Action("deact", Plugin::tr("Deactivate"), [this] { toggle(); })};
        else
            return {Action("act", Plugin::tr("Activate"), [this] { toggle(); })};
    }

    bool isActive() const
    {
        return false;
    }

    void toggle() const
    {

    }

};

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

Plugin::Plugin()
{

    qRegisterMetaType<NestedVariantMap>("NestedVariantMap");
    qDBusRegisterMetaType<NestedVariantMap>();


    auto m = IManager(service, manager_path, QDBusConnection::systemBus());
    for (const auto &conn_path : m.activeConnections())
        WARN << conn_path.path();
}

void Plugin::updateIndexItems()
{
    vector<IndexItem> items;

    auto is = ISettings(service, settings_path, QDBusConnection::systemBus());

    auto reply = is.ListConnections();
    reply.waitForFinished();

    for (auto object_path : reply.value())
    {
        auto ics = IConnectionSettings(service, object_path.path(), QDBusConnection::systemBus());
        auto reply = ics.GetSettings();
        reply.waitForFinished();
        auto cs = reply.value();
        for (const auto&[k,v] : cs.asKeyValueRange())
        CRIT << k << v;

        try {
            auto name = getNestedVariantMapValue<QString>(cs, "connection", "id");
            auto type = getNestedVariantMapValue<QString>(cs, "connection", "type");
            auto uuid = getNestedVariantMapValue<QString>(cs, "connection", "uuid");
            CRIT << "ID: " << uuid << name << type;

            auto item = make_shared<VpnItem>(uuid, name, object_path);

            items.emplace_back(::move(item), name);

        } catch (...) {
            continue;
        }


    }

    setIndexItems(::move(items));
}
