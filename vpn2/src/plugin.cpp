// Copyright (c) 2023-2024 Manuel Schneider

#include "plugin.h"
#include <QApplication>
#include <QMessageBox>
#include <albert/albert.h>
#include <albert/logging.h>
#include "albert/matcher.h"
#include "nm.h"
ALBERT_LOGGING_CATEGORY("vpn")
using namespace albert;
using namespace std;
using IProperties = OrgFreedesktopDBusPropertiesInterface;
using IManager    = OrgFreedesktopNetworkManagerInterface;
using ISettings   = OrgFreedesktopNetworkManagerSettingsInterface;
using IConnection = OrgFreedesktopNetworkManagerSettingsConnectionInterface;

enum class NMActiveConnectionState {
    NM_ACTIVE_CONNECTION_STATE_UNKNOWN,
    NM_ACTIVE_CONNECTION_STATE_ACTIVATING,
    NM_ACTIVE_CONNECTION_STATE_ACTIVATED,
    NM_ACTIVE_CONNECTION_STATE_DEACTIVATING,
    NM_ACTIVE_CONNECTION_STATE_DEACTIVATED
};


class VpnItem :  public Item
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


class NetworkManagerApi : public QObject
{
public:
    static constexpr const char *service = "org.freedesktop.NetworkManager";
    static constexpr const char *object_path_manager = "/org/freedesktop/NetworkManager";
    static constexpr const char *object_path_settings = "/org/freedesktop/NetworkManager/Settings";

    IManager manager;
    ISettings settings;

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

public:

    NetworkManagerApi():
        manager(service, object_path_manager, QDBusConnection::systemBus()),
        settings(service, object_path_settings, QDBusConnection::systemBus())
    {
        for (const auto &conn_path : manager.activeConnections())
            WARN << conn_path.path();

        QDBusConnection::systemBus().connect(
            service, object_path_manager,
            "org.freedesktop.DBus.Properties", // Interface name
            "PropertiesChanged",               // Signal name
            this,
            SLOT(onPropertiesChanged(QString,QVariantMap,QStringList))  // missing spaces intended
            );
    }


    void onPropertiesChanged(const QString &interface,
                             const QVariantMap &changedProperties,
                             const QStringList &invalidatedProperties) {
        CRIT << "Interface:" << interface;
        CRIT << "Changed Properties:" << changedProperties;
        CRIT << "Invalidated Properties:" << invalidatedProperties;
    }



    vector<shared_ptr<VpnItem>> createVpnItems()
    {
        vector<shared_ptr<VpnItem>> items;

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

            try {
                auto name = getNestedVariantMapValue<QString>(conn_settings, "connection", "id");
                auto type = getNestedVariantMapValue<QString>(conn_settings, "connection", "type");
                auto uuid = getNestedVariantMapValue<QString>(conn_settings, "connection", "uuid");

                CRIT << "ID: " << uuid << name << type;

                auto item = make_shared<VpnItem>(uuid, name, object_path);

                items.emplace_back(::move(item));

            } catch (...) {
                continue;
            }
        }

        return items;
    }
};





class Plugin::Private
{
public:
    NetworkManagerApi nm;
    vector<shared_ptr<VpnItem>> items;
};

Plugin::Plugin() : d(make_unique<Private>())
{
    qRegisterMetaType<NestedVariantMap>("NestedVariantMap");
    qDBusRegisterMetaType<NestedVariantMap>();

    d->items = d->nm.createVpnItems();
}

Plugin::~Plugin() = default;

vector<RankItem> Plugin::handleGlobalQuery(const albert::Query &query)
{
    vector<RankItem> matches;
    Matcher matcher(query);
    for (const auto &item : d->items)
        if (auto m = matcher.match(item->text()); m)
            matches.emplace_back(item, m);
    return matches;
}

