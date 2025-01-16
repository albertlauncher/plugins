// Copyright (c) 2020-2024 Ivo Šmerek

#include "plugin.h"

#include <QMessageBox>
#include <QThread>

#include "ui_configwidget.h"
#include <albert/logging.h>
#include <albert/standarditem.h>

#include "spotifyApiClient.h"
#include "albert/util.h"

ALBERT_LOGGING_CATEGORY("spotify")

using namespace albert;


inline auto CFG_CLIENT_ID = "client_id";
inline auto CFG_CLIENT_SECRET = "client_secret";
inline auto CFG_REFRESH_TOKEN = "refresh_token";
inline auto CFG_ALLOW_EXPLICIT = "allow_explicit";
inline auto CFG_NUM_RESULTS = "number_of_results";
inline auto CFG_SPOTIFY_EXECUTABLE = "spotify_executable";
inline auto CFG_CACHE_DIR = "cache_directory";
inline auto CFG_LAST_DEVICE = "last_device";

inline auto DEF_NUM_RESULTS = 5;
inline auto DEF_SPOTIFY_EXECUTABLE = "spotify";


Plugin::Plugin()
{
    const auto credentials = apiCredentials{
        settingsString(CFG_CLIENT_ID),
        settingsString(CFG_CLIENT_SECRET),
        settingsString(CFG_REFRESH_TOKEN),
    };
    api = new SpotifyApiClient(credentials);
}

Plugin::~Plugin()
{
    delete api;
}

QString Plugin::defaultTrigger() const
{
    return "play ";
}

void Plugin::handleTriggerQuery(Query* query)
{
    if (const auto trimmed = query->string().trimmed(); trimmed.isEmpty())
        return;

    if (!query->isValid())
        return;

    // Each query is executed in different thread, so reset the network manager.
    // Read the JSDoc of the method for more information.
    api->resetNetworkManager();

    // If there is no internet connection, make one alerting item to let the user know.
    if (!api->checkServerResponse())
    {
        DEBG << "No internet connection!";
        query->add(StandardItem::make(nullptr, "Can't get an answer from the server.",
                                      "Please, check your internet connection.", nullptr));
        return;
    }

    // If the access token expires, try to refresh it or alert the user what is wrong.
    if (api->isAccessTokenExpired())
    {
        DEBG << "Token expired. Refreshing";
        if (!api->refreshAccessToken())
        {
            query->add(StandardItem::make(nullptr, "Wrong credentials.",
                                          "Please, check the extension settings.", nullptr));
            return;
        }
    }

    // Search for tracks on Spotify using the query.
    const auto tracks = api->searchTracks(query->string(), settingsInt(CFG_NUM_RESULTS, DEF_NUM_RESULTS));

    // Get available Spotify devices.
    const auto devices = api->getDevices();

    const auto activeDevice = findActiveDevice(devices);
    const auto cacheDirectory = settingsString(CFG_CACHE_DIR, QDir::tempPath() + "/albert-spotify-covers");

    if (!QDir(cacheDirectory).exists() && QDir().mkpath(cacheDirectory))
    {
        INFO << "Cache directory created:" << cacheDirectory;
    }

    for (const auto& track : tracks)
    {
        // If the track is explicit and the user doesn't want to see explicit tracks, skip it.
        if (track.isExplicit && !settingsBool(CFG_ALLOW_EXPLICIT)) continue;

        const auto filename = QString("%1/%2.jpeg").arg(cacheDirectory, track.albumId);

        // Download cover image of the album.
        api->downloadFile(track.imageUrl, filename);

        // Create a standard item with a track name in title and album with artists in subtext.
        const auto result = StandardItem::make(
            track.id,
            track.name,
            QString("%1 (%2)").arg(track.albumName, track.artists),
            nullptr,
            {filename});

        auto actions = std::vector<Action>();

        actions.emplace_back(
            "play",
            "Play on Spotify",
            [this, track, activeDevice, devices]
            {
                // Each action is executed in different thread, so reset the network manager.
                // Read the JSDoc of the method for more information.
                api->resetNetworkManager();

                // Check if the last-used device is still available.
                const auto lastDeviceId = settingsString(CFG_LAST_DEVICE);
                const auto lastDeviceAvailable = findDevice(devices, lastDeviceId).id != "";

                if (activeDevice.id != "")
                {
                    // If available, use an active device and play the track.
                    api->playTrack(track, activeDevice.id);
                    INFO << "Playing on active device.";
                    settings()->setValue(CFG_LAST_DEVICE, activeDevice.id);
                }
                else if (lastDeviceAvailable)
                {
                    // If there is not an active device, use last-used one.
                    api->playTrack(track, lastDeviceId);
                    INFO << "Playing on last device.";
                }
                else if (!devices.isEmpty())
                {
                    // Use the first available device.
                    api->playTrack(track, devices[0].id);
                    INFO << "Playing on first found device.";
                    settings()->setValue(CFG_LAST_DEVICE, devices[0].id);
                }
                else
                {
                    // Run local Spotify client, wait until it loads, and play the track.
                    runDetachedProcess(
                        QStringList() << settingsString(
                            CFG_SPOTIFY_EXECUTABLE,
                            DEF_SPOTIFY_EXECUTABLE));
                    api->waitForDeviceAndPlay(track);
                    INFO << "Playing on local Spotify.";
                }
            }
        );

        actions.emplace_back(
            "queue",
            "Add to the Spotify queue",
            [this, track]
            {
                // Each action is executed in different thread, so reset the network manager.
                // Read the JSDoc of the method for more information.
                api->resetNetworkManager();

                api->addTrackToQueue(track);
            }
        );

        // For each device except active create action to transfer Spotify playback to this device.
        for (const auto& device : devices)
        {
            if (device.isActive) continue;

            actions.emplace_back(
                QString("play_on_%1").arg(device.id),
                QString("Play on %1 (%2)").arg(device.type, device.name),
                [this, track, device]
                {
                    // Each action is executed in different thread, so reset the network manager.
                    // Read the JSDoc of the method for more information.
                    api->resetNetworkManager();

                    api->playTrack(track, device.id);
                    settings()->setValue(CFG_LAST_DEVICE, device.id);
                }
            );
        }

        result->setActions(actions);

        query->add(result);
    }
}

QWidget* Plugin::buildConfigWidget()
{
    auto* widget = new QWidget();
    Ui::ConfigWidget ui;
    ui.setupUi(widget);

    ui.lineEdit_client_id->setText(settingsString(CFG_CLIENT_ID));
    connect(ui.lineEdit_client_id,
            &QLineEdit::textEdited,
            this, [this](const QString& value)
            {
                settings()->setValue(CFG_CLIENT_ID, value);
                api->setClientId(value);
            });

    ui.lineEdit_client_secret->setText(settingsString(CFG_CLIENT_SECRET));
    connect(ui.lineEdit_client_secret,
            &QLineEdit::textEdited,
            this, [this](const QString& value)
            {
                settings()->setValue(CFG_CLIENT_SECRET, value);
                api->setClientSecret(value);
            });

    ui.lineEdit_refresh_token->setText(settingsString(CFG_REFRESH_TOKEN));
    connect(ui.lineEdit_refresh_token,
            &QLineEdit::textEdited,
            this, [this](const QString& value)
            {
                settings()->setValue(CFG_REFRESH_TOKEN, value);
                api->setRefreshToken(value);
            });

    // Bind "Test connection" button
    connect(ui.pushButton_test_connection, &QPushButton::clicked, this, [this]
    {
        // UI actions are executed in different thread, so reset the network manager.
        // Read the JSDoc of the method for more information.
        api->resetNetworkManager();

        const bool refreshStatus = api->refreshAccessToken();

        QString message = "Everything is set up correctly.";
        if (!refreshStatus)
        {
            message = api->lastErrorMessage.isEmpty()
                          ? "Can't get an answer from the server.\nPlease, check your internet connection."
                          : QString("Spotify Web API returns: \"%1\"\nPlease, check all input fields.")
                          .arg(api->lastErrorMessage);
        }

        const auto messageBox = new QMessageBox();
        messageBox->setWindowTitle(refreshStatus ? "Success" : "API error");
        messageBox->setText(message);
        messageBox->setIcon(refreshStatus ? QMessageBox::Information : QMessageBox::Critical);
        messageBox->exec();
        delete messageBox;
    });

    ui.checkBox_explicit->setChecked(settingsBool(CFG_ALLOW_EXPLICIT));
    connect(ui.checkBox_explicit,
            &QCheckBox::toggled,
            this, [this](const bool value)
            {
                settings()->setValue(CFG_ALLOW_EXPLICIT, value);
            });

    ui.spinBox_number_of_results->setValue(settingsInt(CFG_NUM_RESULTS, DEF_NUM_RESULTS));
    connect(ui.spinBox_number_of_results,
            &QSpinBox::valueChanged,
            this, [this](const int value)
            {
                settings()->setValue(CFG_NUM_RESULTS, value);
            });

    ui.lineEdit_spotify_executable->setText(settingsString(CFG_SPOTIFY_EXECUTABLE));
    connect(ui.lineEdit_spotify_executable,
            &QLineEdit::textEdited,
            this, [this](const QString& value)
            {
                if (value.isEmpty())
                {
                    settings()->remove(CFG_SPOTIFY_EXECUTABLE);
                    return;
                }
                settings()->setValue(CFG_SPOTIFY_EXECUTABLE, value);
            });

    ui.lineEdit_cache_directory->setText(settingsString(CFG_CACHE_DIR));
    connect(ui.lineEdit_cache_directory,
            &QLineEdit::textEdited,
            this, [this](const QString& value)
            {
                if (value.isEmpty())
                {
                    settings()->remove(CFG_CACHE_DIR);
                    return;
                }
                settings()->setValue(CFG_CACHE_DIR, value);
            });

    return widget;
}

Device Plugin::findActiveDevice(const QVector<Device>& devices)
{
    for (const auto& device : devices)
    {
        if (device.isActive)
        {
            return device;
        }
    }

    return {};
}

Device Plugin::findDevice(const QVector<Device>& devices, const QString& id)
{
    for (const auto& device : devices)
    {
        if (device.id == id)
        {
            return device;
        }
    }

    return {};
}

QString Plugin::settingsString(const QAnyStringView key) const
{
    return settings()->value(key).toString();
}

QString Plugin::settingsString(const QAnyStringView key, const QVariant& defaultValue) const
{
    return settings()->value(key, defaultValue).toString();
}

int Plugin::settingsInt(const QAnyStringView key) const
{
    return settings()->value(key).toInt();
}

int Plugin::settingsInt(const QAnyStringView key, const QVariant& defaultValue) const
{
    return settings()->value(key, defaultValue).toInt();
}

bool Plugin::settingsBool(const QAnyStringView key) const
{
    return settings()->value(key).toBool();
}

bool Plugin::settingsBool(const QAnyStringView key, const QVariant& defaultValue) const
{
    return settings()->value(key, defaultValue).toBool();
}
