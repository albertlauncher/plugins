// Copyright (c) 2020-2024 Ivo Šmerek

#pragma once
#include <albert/extensionplugin.h>
#include "albert/property.h"
#include "albert/triggerqueryhandler.h"

#include "spotifyApiClient.h"


class Plugin final : public albert::ExtensionPlugin,
                     public albert::TriggerQueryHandler
{
    ALBERT_PLUGIN

public:
    Plugin();
    ~Plugin() override;

private:
    SpotifyApiClient* api;

    QString defaultTrigger() const override;
    void handleTriggerQuery(albert::Query*) override;
    QWidget* buildConfigWidget() override;

    /**
     * Find the active device from a list of devices.
     * @param devices The list of devices to search.
     * @return The active device, or an empty device if none is active.
     */
    static Device findActiveDevice(const QVector<Device>& devices);

    /**
     * Find a device by ID from a list of devices.
     * @param devices The list of devices to search.
     * @param id The ID of the device to find.
     * @return The device with the given ID, or an empty device if none is found.
     */
    static Device findDevice(const QVector<Device>& devices, const QString& id);

    // Helper functions for accessing settings

    QString settingsString(QAnyStringView key) const;
    QString settingsString(QAnyStringView key, const QVariant& defaultValue) const;
    int settingsInt(QAnyStringView key) const;
    int settingsInt(QAnyStringView key, const QVariant& defaultValue) const;
    bool settingsBool(QAnyStringView key) const;
    bool settingsBool(QAnyStringView key, const QVariant& defaultValue) const;
};
