// Copyright (c) 2023-2024 Manuel Schneider

#include "albert/albert.h"
#include "albert/extension/queryhandler/item.h"
#include "albert/logging.h"
#include "plugin.h"
#include <QApplication>
#include <QMessageBox>
#include <SystemConfiguration/SystemConfiguration.h>
ALBERT_LOGGING_CATEGORY("vpn")
using namespace albert;
using namespace std;


// TODO perfect use case for dynamic items
class SCNetworkInterfaceItem : public Item
{
  QString service_id;
  QString service_name;
  SCNetworkConnectionRef connection;

public:
    SCNetworkInterfaceItem(CFStringRef service_id_, SCNetworkServiceRef service)
    {
        service_id = QString::fromCFString(service_id_);
        service_name = QString::fromCFString(SCNetworkServiceGetName(service));
        connection = SCNetworkConnectionCreateWithServiceID(NULL, service_id_, NULL, NULL);
    }

    ~SCNetworkInterfaceItem()
    {
        CFRelease(connection);
    }

    SCNetworkConnectionStatus status() const
    {
        return SCNetworkConnectionGetStatus(connection);
    }

    QString statusString() const
    {
        switch (status()) {
        case kSCNetworkConnectionInvalid:
            return Plugin::tr("Invalid");
        case kSCNetworkConnectionDisconnected:
            return Plugin::tr("Disconnected");
        case kSCNetworkConnectionConnecting:
            return Plugin::tr("Connecting…");
        case kSCNetworkConnectionConnected:
            return Plugin::tr("Connected");
        case kSCNetworkConnectionDisconnecting:
            return Plugin::tr("Disconnecting…");
        default:
            return Plugin::tr("Unknown");
        }
    }

    void setConnected(bool connect) const
    {
        if (connect)
        {
            if (SCNetworkConnectionStart(connection, NULL, TRUE))  // stay open even on app exit
                INFO << "Successfully started connecting:" << service_name;
            else
            {
                auto msg = Plugin::tr("Failed connecting '%1': %2.")
                               .arg(service_name, SCErrorString(SCError()));
                WARN << msg;
                QMessageBox::warning(nullptr, qApp->applicationDisplayName(), msg);
            }
        }
        else
        {
            if (SCNetworkConnectionStop(connection, TRUE))  // force stop
                INFO << QString("Successfully stopped connection: %1").arg(service_name);
            else
            {
                auto msg = Plugin::tr("Failed disconnecting '%1': %2.")
                               .arg(service_name, SCErrorString(SCError()));
                WARN << msg;
                QMessageBox::warning(nullptr, qApp->applicationDisplayName(), msg);
            }
        }
    }

    QString id() const { return service_id; }

    QString text() const { return service_name; }

    QString subtext() const { return statusString(); }

    QStringList iconUrls() const {
        switch (status()) {
        case kSCNetworkConnectionInvalid:
            return {QStringLiteral("gen:?&text=VPN&background=#D00000&foreground=#ffffff&fontscalar=0.4")};
        case kSCNetworkConnectionDisconnected:
            return {QStringLiteral("gen:?&text=VPN&background=#808080&foreground=#ffffff&fontscalar=0.4")};
        case kSCNetworkConnectionConnecting:
            return {QStringLiteral("gen:?&text=VPN&background=#00D0D0&foreground=#ffffff&fontscalar=0.4")};
        case kSCNetworkConnectionConnected:
            return {QStringLiteral("gen:?&text=VPN&background=#00D000&foreground=#ffffff&fontscalar=0.4")};
        case kSCNetworkConnectionDisconnecting:
            return {QStringLiteral("gen:?&text=VPN&background=#D0D000&foreground=#ffffff&fontscalar=0.4")};
        default:
            return {":unknown"};
        }
    }

    QString inputActionText() const { return service_name; }

    vector<Action> actions() const
    {
        switch (status()) {
        case kSCNetworkConnectionConnected:
            return {
                {
                    QStringLiteral("disconnect"),
                    Plugin::tr("Disconnect"),
                    [this](){ setConnected(false);}
                }
            };
        case kSCNetworkConnectionDisconnected:
            return {
                {
                    QStringLiteral("connect"),
                    Plugin::tr("Connect"),
                    [this](){ setConnected(true);}
                }
            };
        default:
            return {};
        }
    }
};

QString Plugin::synopsis() const
{
    const static auto tr_s = tr("<VPN name>");
    return tr_s;
}

void Plugin::updateIndexItems()
{
    vector<IndexItem> items;

    // Iterate network services in the default system preferences
    SCPreferencesRef preferences = SCPreferencesCreate(NULL, CFSTR("albert"), NULL);
    if (preferences) {

        CFArrayRef services = SCNetworkServiceCopyAll(preferences);
        if (services) {

            for (CFIndex i = 0; i < CFArrayGetCount(services); i++){
                auto service = (SCNetworkServiceRef)CFArrayGetValueAtIndex(services, i);

                // If enabled
                if (SCNetworkServiceGetEnabled(service)){

                    // If has service id
                    CFStringRef service_id = SCNetworkServiceGetServiceID(service);
                    if (service_id){

                        // If has interface
                        SCNetworkInterfaceRef interface = SCNetworkServiceGetInterface(service);
                        if (interface){

                            // If interface type is VPN
                            CFStringRef interface_type = SCNetworkInterfaceGetInterfaceType(interface);
                            if (CFStringCompare(interface_type, CFSTR("VPN"), 0) == kCFCompareEqualTo
                                || CFStringCompare(interface_type, CFSTR("IPSec"), 0) == kCFCompareEqualTo)
                            {
                                auto item = static_pointer_cast<Item>(make_shared<SCNetworkInterfaceItem>(service_id, service));
                                items.emplace_back(item, QString::fromCFString(SCNetworkServiceGetName(service)));
                            }
                            else
                                DEBG << "Skipping interface type" << interface_type << service_id;
                        }
                    }
                }
            }
            CFRelease(services);
        }
        CFRelease(preferences);
    }

    setIndexItems(::move(items));
}

