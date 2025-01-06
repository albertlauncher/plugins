// Copyright (c) 2024 Manuel Schneider

#include "plugin.h"
#include <CoreLocation/CoreLocation.h>
#include <CoreWLAN/CWInterface.h>
#include <CoreWLAN/CoreWLAN.h>
#include <IOBluetooth/IOBluetooth.h>
#include <Security/Security.h>
#include <albert/logging.h>
#include <albert/matcher.h>
#include <albert/notification.h>
#include <albert/standarditem.h>
#include <albert/util.h>
ALBERT_LOGGING_CATEGORY("wifi")
using namespace albert;
using namespace std;
#if  ! __has_feature(objc_arc)
#error This file must be compiled with ARC.
#endif

static const char *settings_url = "x-apple.systempreferences:com.apple.wifi-settings-extension";

@interface LocationManagerDelegate : NSObject <CLLocationManagerDelegate>
@end

@implementation LocationManagerDelegate
- (void)locationManager:(CLLocationManager *)manager didChangeAuthorizationStatus:(CLAuthorizationStatus)status {
    if (status == kCLAuthorizationStatusAuthorizedAlways) {
        INFO << "Location access granted.";
    } else {
        INFO << "Location access denied.";
    }
}
@end


class Plugin::Private
{
public:
    QString tr_wifi = Plugin::tr("WiFi");
    static const QStringList icon_urls;
    bool fuzzy;
    CLLocationManager *locman = nil;
    LocationManagerDelegate *delegate = nil;
};



const QStringList Plugin::Private::icon_urls =
    {"qfip:/System/Applications/Utilities/AirPort Utility.app"};




Plugin::Plugin() : d(make_unique<Private>())
{
    d->locman = [[CLLocationManager alloc] init];
    d->delegate = [[LocationManagerDelegate alloc] init];
    d->locman.delegate = d->delegate;

    [d->locman requestAlwaysAuthorization];

    // if ([CLLocationManager authorizationStatus] != kCLAuthorizationStatusAuthorizedAlways)
    //     throw std::runtime_error(tr("Wi-Fi details require location access. "
    //                                 "Please enable it in System Preferences.").toStdString());
}

Plugin::~Plugin() = default;

QString Plugin::defaultTrigger() const { return QStringLiteral("wifi "); }

bool Plugin::supportsFuzzyMatching() const { return true; }

void Plugin::setFuzzyMatching(bool val) { d->fuzzy = val; }

static NSString *getPasswordForSSID(NSString *ssid) {
    UInt32 passwordLength;
    void *passwordData = nullptr;
    OSStatus status = SecKeychainFindGenericPassword(
        nullptr,                                   // Default keychain
        (UInt32)ssid.length,                      // SSID length
        [ssid UTF8String],                        // SSID
        0, nullptr,                               // Account (not used for Wi-Fi)
        &passwordLength,                          // Output: Password length
        &passwordData,                            // Output: Password data
        nullptr                                   // Item reference (not needed)
        );

    if (status == noErr) {
        NSString *password = [[NSString alloc] initWithBytes:passwordData
                                                      length:passwordLength
                                                    encoding:NSUTF8StringEncoding];
        SecKeychainItemFreeContent(nullptr, passwordData); // Free memory
        return password;
    } else
        return nil;
}

vector<RankItem> Plugin::handleGlobalQuery(const Query *q)
{
    CWInterface *wifi = [[CWWiFiClient sharedWiFiClient] interface];

    if (!wifi)
        WARN << "No Wi-Fi interface found.";

    if ([d->locman authorizationStatus] != kCLAuthorizationStatusAuthorizedAlways)
    {
        WARN << "Wi-Fi details require location access. Enable it in System Preferences.";
        return {};
    }

    vector<RankItem> r;
    Matcher matcher(q->string(), {.fuzzy = d->fuzzy});


    // Wi-Fi item

    if (auto m = matcher.match(d->tr_wifi))
    {
        r.emplace_back(
            StandardItem::make(
                id(),
                d->tr_wifi,
                wifi.powerOn ? tr("Enabled") : tr("Disabled"),
                d->icon_urls,
                {
                    {
                        QStringLiteral("pow"), wifi.powerOn ? tr("Disable") : tr("Enable"),
                        [wifi] {
                            NSError *error = nil;
                            if (![wifi setPower:!wifi.powerOn error:&error])
                                WARN << "Failed to toggle WiFi: "
                                     << error.localizedDescription.UTF8String;
                        }
                    },
                    {
                        QStringLiteral("sett"), tr("Open settings"), [] { openUrl(settings_url); }
                    }
                }
            ),
            m
        );
    }

    if (!wifi.powerOn)
        return r;


    // Network items

    // CWConfiguration *config = [wifi configuration];
    // if (!config) {
    //     WARN << "Failed to get configuration.";
    //     return r;
    // }

    // WARN << "Known networks:";
    // for (CWNetworkProfile *profile in config.networkProfiles)
    // {
    //     profile.
    //     WARN <<  profile.ssid << (long)profile.security;
    // }




    // Add networks
    for (CWNetwork *network in wifi.cachedScanResults)
        @autoreleasepool
        {
            auto ssid = QString::fromNSString(network.ssid);
            CRIT << ssid;

            if (auto m = matcher.match(ssid))
            {
                if ([wifi.ssid isEqualToString:network.ssid])  // Currently connected network
                {
                    r.emplace_back(
                        StandardItem::make(
                            id(), ssid, tr("Connected"), d->icon_urls,
                            {{
                                QStringLiteral("disconnect"), tr("Disconnect"),
                                [=] { [wifi disassociate]; }
                            }}
                            ),
                        m.score()
                    );
                }
                else
                {
                    r.emplace_back(
                        StandardItem::make(
                            id(), ssid, tr("Available"), d->icon_urls,
                            {{
                                QStringLiteral("connect"), tr("Connect"), [=]{

                                    NSString *password = getPasswordForSSID(network.ssid);
                                    if (!password){
                                        WARN << "Failed to retrieve password for SSID: " << ssid;
                                        return;
                                    }

                                    NSError *error = nil;
                                    if (![wifi associateToNetwork:network
                                                         password:password
                                                            error:&error])
                                        WARN << QString("Failed to connect to '%1': %2")
                                                    .arg(ssid, QString::fromNSString(error.localizedDescription));
                                    else
                                        INFO << QString("Successfully connected to '%1'.").arg(ssid);
                                }
                            }}
                            ),
                        m.score()
                    );
                }
            }
        }

    return r;
}


