// Copyright (c) 2023-2025 Manuel Schneider

#include "item.h"
#include "plugin.h"
#include <QApplication>
#include <QMessageBox>
#include <SystemConfiguration/SystemConfiguration.h>
#include <albert/albert.h>
#include <albert/logging.h>
using namespace albert;
using namespace std;

static VpnConnectionItem::State toState(SCNetworkConnectionStatus status)
{
    switch (status) {
    case kSCNetworkConnectionInvalid:
        return VpnConnectionItem::State::Invalid;
    case kSCNetworkConnectionDisconnected:
        return VpnConnectionItem::State::Disconnected;
    case kSCNetworkConnectionConnecting:
        return VpnConnectionItem::State::Connecting;
    case kSCNetworkConnectionConnected:
        return VpnConnectionItem::State::Connected;
    case kSCNetworkConnectionDisconnecting:
        return VpnConnectionItem::State::Disconnecting;
    }
    return VpnConnectionItem::State::Invalid;
}

class SCNetworkInterfaceItem::Private
{
public:
    QString service_id;
    QString service_name;
    SCNetworkConnectionRef connection;
    SCNetworkConnectionContext context{0, NULL, NULL, NULL, NULL};
};

static void connectionCallback(SCNetworkConnectionRef /*connection*/, SCNetworkConnectionStatus status, void *info)
{
    static_cast<SCNetworkInterfaceItem *>(info)->setState(toState(status));
}

SCNetworkInterfaceItem::SCNetworkInterfaceItem(const QString &service_id,
                                               const QString &service_name):
    d(make_unique<Private>())
{
    d->service_id = service_id;
    d->service_name = service_name;
    d->context.info = this;
    d->connection = SCNetworkConnectionCreateWithServiceID(NULL,
                                                           service_id.toCFString(),
                                                           connectionCallback,
                                                           &d->context);

    if (!d->connection)
        throw runtime_error("SCNetworkConnectionCreateWithServiceID failed!");

    CFRunLoopRef runLoop = CFRunLoopGetCurrent();
    if (!SCNetworkConnectionScheduleWithRunLoop(d->connection, runLoop, kCFRunLoopCommonModes))
        throw runtime_error("Failed to schedule SCNetworkConnection with CFRunLoop!");

    // Set initial state
    setState(toState(SCNetworkConnectionGetStatus(d->connection)));
}

SCNetworkInterfaceItem::~SCNetworkInterfaceItem()
{
    CFRelease(d->connection);
}

void SCNetworkInterfaceItem::setConnected(bool connect) const
{
    if (connect) {
        if (SCNetworkConnectionStart(d->connection, NULL, TRUE)) // stay open even on app exit
            INFO << "Successfully started connecting:" << d->service_name;
        else {
            auto msg = Plugin::tr("Failed connecting '%1': %2.")
                           .arg(d->service_name, SCErrorString(SCError()));
            WARN << msg;
            QMessageBox::warning(nullptr, qApp->applicationDisplayName(), msg);
        }
    } else {
        if (SCNetworkConnectionStop(d->connection, TRUE)) // force stop
            INFO << QString("Successfully stopped connection: %1").arg(d->service_name);
        else {
            auto msg = Plugin::tr("Failed disconnecting '%1': %2.")
                           .arg(d->service_name, SCErrorString(SCError()));
            WARN << msg;
            QMessageBox::warning(nullptr, qApp->applicationDisplayName(), msg);
        }
    }
}

QString SCNetworkInterfaceItem::id() const { return d->service_id; }

QString SCNetworkInterfaceItem::text() const { return d->service_name; }

vector<Action> SCNetworkInterfaceItem::actions() const
{
    switch (state()) {
    case VpnConnectionItem::State::Connected:
        return {{QStringLiteral("disconnect"), Plugin::tr("Disconnect"),
                 [this] { setConnected(false); }}};
    case VpnConnectionItem::State::Disconnected:
        return {{QStringLiteral("connect"), Plugin::tr("Connect"),
                 [this] { setConnected(true); }}};
    default:
        return {};
    }
}

vector<shared_ptr<Item>> SCNetworkInterfaceItem::createItems()
{
    vector<shared_ptr<Item>> items;

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
                                items.push_back(make_shared<SCNetworkInterfaceItem>(
                                    QString::fromCFString(service_id),
                                    QString::fromCFString(SCNetworkServiceGetName(service))
                                    ));
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

    return items;
}
