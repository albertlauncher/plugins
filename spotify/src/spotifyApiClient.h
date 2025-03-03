// Copyright (c) 2020-2024 Ivo Šmerek

#pragma once
#include <qreadwritelock.h>
#include <QtNetwork/QNetworkAccessManager>

#include "types/device.h"
#include "types/track.h"


inline QString TOKEN_URL = "https://accounts.spotify.com/api/token";
inline QString SEARCH_URL = "https://api.spotify.com/v1/search?q=%1&type=%2&limit=%3";
inline QString DEVICES_URL = "https://api.spotify.com/v1/me/player/devices";
inline QString QUEUE_URL = "https://api.spotify.com/v1/me/player/queue?uri=%1";
inline QString PLAY_URL = "https://api.spotify.com/v1/me/player/play?device_id=%1";

inline int DEFAULT_TIMEOUT = 10000;

struct apiCredentials
{
    QString clientId;
    QString clientSecret;
    QString refreshToken;
};


/**
 * Spotify API client for interacting with the Spotify Web API.
 */
class SpotifyApiClient final : public QObject
{
public:
    /** Contains string description of the last error message. */
    QString lastErrorMessage;

    void setClientId(const QString& id) { clientId = id; }
    void setClientSecret(const QString& secret) { clientSecret = secret; }
    void setRefreshToken(const QString& token) { refreshToken = token; }

    explicit SpotifyApiClient(const apiCredentials& credentials);
    ~SpotifyApiClient() override;

    /**
     * Reset the network manager. This is necessary because the network manager
     * is not guaranteed to be thread-safe and shouldn't be shared between threads.
     * Call this method each time in a new thread.
     * @see https://stackoverflow.com/questions/35684123
     */
    void resetNetworkManager();

    /**
     * Check if the access token is expired.
     * @return true if the access token is expired, false otherwise.
     */
    bool isAccessTokenExpired() const;

    // WEB API CALLS //

    /**
     * Request and store a new access token from Spotify.
     * @return true if the accessToken was successfully refreshed.
     */
    bool refreshAccessToken();

    /**
     * Check response of Spotify API server.
     * @return true if server returns any response, false otherwise.
     */
    bool checkServerResponse() const;

    /**
     * Download a file from the given URL and save it to the given file path.
     * It will not download the file if the given pilePath already exists.
     * @param url URL to download.
     * @param filePath File path to save the file to.
     */
    void downloadFile(const QString& url, const QString& filePath);

    /**
     * Search for tracks on Spotify.
     * @param query The search query.
     * @param limit The maximum number of tracks to return.
     * @return A list of tracks found by the search.
     */
    QVector<Track> searchTracks(const QString& query, int limit);

    /**
     * Returns list of users available Spotify devices.
     */
    QVector<Device> getDevices();

    /**
     * Wait for any device to be ready.
     * @param track
     */
    void waitForDevice(const Track& track);

    /**
     * Wait for any device to be ready and play a track on it.
     * @param track The track object to play.
     */
    void waitForDeviceAndPlay(const Track& track);

    /**
     * Add a track to the queue of a specific device.
     * @param track The track object to add to the queue.
     */
    void addTrackToQueue(const Track& track) const;

   public slots:
    /**
     * Play a track on a specific device.
     * @param track The track object to play.
     * @param deviceId The ID of the device to play the track on.
     */
    void playTrack(const Track& track, const QString& deviceId) const;

private:
    Q_OBJECT

    /** Network manager for sending requests. */
    QNetworkAccessManager* manager = nullptr;

    QString clientId;
    QString clientSecret;
    QString refreshToken;
    QString accessToken;

    QDateTime expirationTime;
    QReadWriteLock fileLock;

    /**
     * Wait for a specific signal from an object.
     * @param sender The object emitting the signal.
     * @param signal The signal to wait for.
     */
    static void waitForSignal(const QObject* sender, const char* signal);

    /**
     * Convert a JSON string to a JSON object.
     * @param string The JSON string to convert.
     * @return The JSON object.
     */
    static QJsonObject stringToJson(const QString& string);

    /**
     * Create a network request with the given URL.
     * @param url The URL to create the request for.
     * @return The created request with access token from instance.
     */
    QNetworkRequest createRequest(const QUrl& url) const;

    /**
     * @return true if the network manager is ready for current thread.
     */
    bool isNetworkManagerSafe() const;

    /**
     * Parse a JSON object to a device object.
     * @param deviceData The JSON object to parse.
     * @return The parsed device object.
     */
    static Device parseDevice(QJsonObject deviceData);

    /**
     * Parse a JSON object to a track object.
     * @param trackData The JSON object to parse.
     * @return The parsed track object.
     */
    static Track parseTrack(QJsonObject trackData);

    /**
     * Linearize a list of artists to a single string.
     * @param artists The list of artists to linearize.
     * @return String of artists separated by commas.
     */
    static QString linearizeArtists(const QJsonArray& artists);

signals:
    void deviceReady(const Track&, QString);
};
