// Copyright (C) 2020-2021 Ivo Šmerek

#pragma once
#include <QDateTime>
#include <QJsonArray>
#include <QString>
#include <QObject>
#include <QReadWriteLock>
#include <QtNetwork/QNetworkAccessManager>
#include "track.h"
#include "device.h"

namespace Spotify {

class SpotifyWebAPI : public QObject {
Q_OBJECT

private:
    QString TOKEN_URL = "https://accounts.spotify.com/api/token";
    QString SEARCH_URL = "https://api.spotify.com/v1/search?q=%1&type=%2&limit=%3";
    QString PLAY_URL = "https://api.spotify.com/v1/me/player/play?device_id=%1";
    QString ADD_ITEM_URL = "https://api.spotify.com/v1/me/player/queue?uri=%1";
    QString DEVICES_URL = "https://api.spotify.com/v1/me/player/devices";

    QString clientId_;
    QString clientSecret_;
    QString refreshToken_;
    QString accessToken_;
    QDateTime expirationTime_;
    QReadWriteLock fileLock_;

    // Helper function for parsing JSON from HTTP answer.
    static QJsonObject answerToJson_(const QString& answer);

    // Helper function for waiting for signal.
    static void waitForSignal_(const QObject *sender, const char *signal);

    // Helper function for waiting and signalling.
    QString waitForDevice_(QString uri, int timeout);

    QNetworkRequest buildRequest_(const QUrl& url);

public:
    explicit SpotifyWebAPI(QObject* parent);
    ~SpotifyWebAPI() override;

    QNetworkAccessManager *manager;
    QString lastErrorMessage;
    Device *activeDevice = nullptr;

    // Tests internet connection to Spotify servers.
    bool testInternetConnection();

    // Set Web API credentials. Use testConnection for check.
    void setConnection(QString clientId, QString clientSecret, QString refreshToken);

    // Try to refresh the access_token and return true if successful.
    bool refreshToken();

    // Does the same as refreshToken. Is here for better code readability.
    // In case it returns false, you can find error message in lastErrorMessage variable.
    bool testConnection();

    // Returns true if the access_token has expired. Use refreshToken function to get new token.
    bool expired();

    // Returns QVector with tracks matching the search query.
    QVector<Track> searchTracks(const QString& query, int limit);

    // Downloads image from imageUrl to imageFilePath.
    void downloadImage(const QString& imageUrl, const QString& imageFilePath);

    // Adds track to Spotify listening queue.
    void addItemToQueue(const QString& uri);

    // Asynchronously plays the song as soon as an available device appears.
    // Gives up after the timeout.
    QString waitForDeviceAndPlay(const QString& uri, int timeout);

    // Returns list of users available Spotify devices.
    QVector<Device> *getDevices();

    // Returns id of first available Spotify device.
    QString getFirstDeviceId();

signals:
    void deviceReady(QString, QString);

public slots:
    // Plays track on Spotify device.
    void play(const QString& uri, QString device = "");
};

}