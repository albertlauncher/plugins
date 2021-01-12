// Copyright (C) 2020-2021 Ivo Šmerek

#pragma once
#include <QDateTime>
#include <QJsonArray>
#include <QString>
#include <QObject>
#include <QtNetwork/QNetworkAccessManager>
#include "track.h"
#include "device.h"

namespace Spotify {

class SpotifyWebAPI : public QObject {
Q_OBJECT

public:
    SpotifyWebAPI();
    ~SpotifyWebAPI() override;

    QNetworkAccessManager *manager;
    QString lastErrorMessage;
    Device *activeDevice;

    // Set Web API credentials. Use testConnection for check.
    void setConnection(QString clientId, QString clientSecret, QString refreshToken);

    // This method will try to refresh the access_token and return true if successful.
    bool refreshToken();

    // Does the same as refreshToken. Is here for better code readability.
    // In case it returns false, you can find error message in lastErrorMessage variable.
    bool testConnection();

    // Returns true if the access_token has expired. Use refreshToken function to get new token.
    bool expired();

    // Returns QVector with tracks matching the search query.
    QVector<Track> searchTrack(const QString& query, const QString& limit);

    // This method downloads image from imageUrl to imageFilePath.
    void downloadImage(const QString& imageUrl, const QString& imageFilePath);

    void addItemToQueue(const QString& uri);

    void skipToNextTrack();

    void play(const QString& uri, QString device = "");

    QVector<Device> getDevices();

private:
    QString TOKEN_URL = "https://accounts.spotify.com/api/token";
    QString SEARCH_URL = "https://api.spotify.com/v1/search?q=%1&type=%2&limit=%3";
    QString PLAY_URL = "https://api.spotify.com/v1/me/player/play?device_id=%1";
    QString ADD_ITEM_URL = "https://api.spotify.com/v1/me/player/queue?uri=%1";
    QString NEXT_TRACK_URL = "https://api.spotify.com/v1/me/player/next";
    QString DEVICES_URL = "https://api.spotify.com/v1/me/player/devices";

    QString clientId_;
    QString clientSecret_;
    QString refreshToken_;
    QString accessToken_;
    QDateTime expirationTime_;
    QJsonArray itemResults_;
    QJsonArray devicesResult_;

    static QJsonObject answerToJson_(const QString& answer);

signals:
    void tokenRefreshed();
    void tokenReplyReceived();
    void searchReplyReceived();
    void imageReceived();
    void addedToQueue();
    void skippedTrack();
    void played();
};

}