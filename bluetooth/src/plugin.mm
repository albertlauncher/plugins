// Copyright (c) 2024 Manuel Schneider

#include "plugin.h"
#include <IOBluetooth/IOBluetooth.h>
#include <QTimer>
#include <albert/logging.h>
#include <albert/matcher.h>
#include <albert/notification.h>
#include <albert/standarditem.h>
#include <albert/util.h>
ALBERT_LOGGING_CATEGORY("bluetooth")
using namespace albert;
using namespace std;
#if  ! __has_feature(objc_arc)
#error This file must be compiled with ARC.
#endif


extern "C" {
int IOBluetoothPreferencesAvailable();
int IOBluetoothPreferenceGetControllerPowerState();
void IOBluetoothPreferenceSetControllerPowerState(int state);
int IOBluetoothPreferenceGetDiscoverableState();
void IOBluetoothPreferenceSetDiscoverableState(int state);
}


@interface BluetoothConnectionHandler : NSObject
- (void)connectionComplete:(IOBluetoothDevice *)device status:(IOReturn)status;
@end


class Plugin::Private
{
public:
    QString tr_bt = Plugin::tr("Bluetooth");
    static const QStringList icon_urls;
    __strong BluetoothConnectionHandler* delegate;
    bool fuzzy;
};

const QStringList Plugin::Private::icon_urls =
    {"qfip:/System/Applications/Utilities/Bluetooth File Exchange.app"};


Plugin::Plugin() : d(make_unique<Private>())
{
    d->delegate = [[BluetoothConnectionHandler alloc] init];

    // Touch once to request permissions
    IOBluetoothPreferenceGetControllerPowerState();
}

Plugin::~Plugin() = default;

QString Plugin::defaultTrigger() const { return QStringLiteral("bt "); }

bool Plugin::supportsFuzzyMatching() const { return true; }

void Plugin::setFuzzyMatching(bool val) { d->fuzzy = val; }

vector<RankItem> Plugin::handleGlobalQuery(const Query *q)
{
    vector<RankItem> r;

    bool enabled = IOBluetoothPreferenceGetControllerPowerState();

    Matcher matcher(q->string(), {.fuzzy = d->fuzzy});

    if (auto m = matcher.match(d->tr_bt))
    {
        auto desc = enabled ? tr("Enabled") : tr("Disabled");
        r.emplace_back(StandardItem::make(
            id(), d->tr_bt, desc, d->icon_urls,
            {
                {
                    QStringLiteral("pow"), enabled ? tr("Disable") : tr("Enable"),
                    [=]{ IOBluetoothPreferenceSetControllerPowerState(enabled ? 0 : 1); }
                },
                {
                    QStringLiteral("sett"), tr("Open settings"),
                    [=]{ openUrl("x-apple.systempreferences:com.apple.Bluetooth"); }
                }
            }),
            m.score()
        );
    }

    if (!enabled)
        return r;

    for (IOBluetoothDevice *device : [IOBluetoothDevice pairedDevices])
        @autoreleasepool
    {
        auto device_name = QString::fromNSString(device.name);
        if (auto m = matcher.match(device_name))
        {
            auto desc = device.isConnected
                    ? tr("Connected")
                    : tr("Not connected");

            auto action = [=, this]{
                if (device.isConnected)
                {
                    auto status = [device closeConnection];
                    if (status == kIOReturnSuccess)
                        INFO <<  QString("Successfully disconnected '%1'.").arg(device_name);
                    else
                        WARN <<  QString("Failed disconnecting '%1': Status '%2'")
                                 .arg(device_name, status);
                }
                else
                {
                    auto status = [device openConnection:d->delegate];
                    if (status == kIOReturnSuccess)
                        INFO << QString("Successfully issued CREATE_CONNECTION command for '%1'.")
                                .arg(device_name);
                    else
                        WARN << QString("Failed issuing CREATE_CONNECTION command for '%1': Status '%2'")
                                .arg(device_name, status);
                }
            };

            r.emplace_back(
                StandardItem::make(
                    id(), device_name, desc, d->icon_urls,
                    {{
                        QStringLiteral("toogle"),
                        device.isConnected ? tr("Disconnect") : tr("Connect"),
                        action
                    }}
                ),
                m.score()
            );
        }
    }
    return r;
}


// Do not move upwards
// Interface implenmentation before plugin implementation confuses lupdate

@implementation BluetoothConnectionHandler

- (void)connectionComplete:(IOBluetoothDevice *)device status:(IOReturn)status
{
    auto name = QString::fromNSString(device.name);
    if (status == kIOReturnSuccess)
        INFO << QString("Successfully connected to device: '%1'").arg(name);
    else
    {
        WARN << QString("Failed to connect to device: '%1', status: '%2'")
                .arg(name).arg(status);

        auto *n = new Notification(
            name,
            Plugin::tr("Connection failed")
        );
        QTimer::singleShot(5000, n, [n]{ n->deleteLater(); });
        n->send();
    }
}

@end
