// Copyright (C) 2020-2021 Ivo Šmerek

#include <QtCore/QJsonArray>
#include <QtCore/QJsonDocument>
#include <QtCore/QJsonObject>
#include <QtNetwork/QNetworkReply>
#include <QtCore/QEventLoop>
#include <QtCore/QFile>
#include <QtCore/QSaveFile>
#include <utility>
#include "spotifyWebAPI.h"

namespace Spotify {

    SpotifyWebAPI::SpotifyWebAPI() {
        manager = new QNetworkAccessManager();
    }

    SpotifyWebAPI::~SpotifyWebAPI() = default;

    QJsonObject SpotifyWebAPI::answerToJson_(const QString& answer) {
        QJsonDocument doc = QJsonDocument::fromJson(answer.toUtf8());
        QJsonObject jsonObject = doc.object();
        return jsonObject;
    }

    void SpotifyWebAPI::setConnection(QString clientId, QString clientSecret, QString refreshToken) {
        clientId_ = std::move(clientId);
        clientSecret_ = std::move(clientSecret);
        refreshToken_ = std::move(refreshToken);
    }

    bool SpotifyWebAPI::refreshToken() {
        auto *url = new QUrl(TOKEN_URL);
        QNetworkRequest request(*url);

        auto hash = QString("%1:%2").arg(clientId_, clientSecret_).toUtf8().toBase64();
        auto header = QString("Basic ").append(hash);
        request.setRawHeader(QByteArray("Authorization"), header.toUtf8());
        request.setHeader(QNetworkRequest::ContentTypeHeader,
                          QVariant(QString("application/x-www-form-urlencoded")));

        QByteArray postData = QString("grant_type=refresh_token&refresh_token=%1").arg(refreshToken_).toLocal8Bit();

        QString savedToken = accessToken_;

        QNetworkReply *reply = manager->post(request, postData);

        connect(reply, &QNetworkReply::finished, [this, reply]() {
            QString answer = reply->readAll();
            QJsonObject jsonVariant = answerToJson_(answer);

            qDebug() << answer;

            accessToken_ = "";

            if (!jsonVariant["access_token"].isUndefined()) {
                accessToken_ = jsonVariant["access_token"].toString();
                qDebug() << "Expires in " << jsonVariant["expires_in"];
                expirationTime_ = QDateTime::currentDateTime().addSecs(jsonVariant["expires_in"].toInt());

                emit tokenReplyReceived();
                emit tokenRefreshed();
            }

            if (!jsonVariant["error_description"].isUndefined()) {
                lastErrorMessage = jsonVariant["error_description"].toString();
            } else {
                lastErrorMessage = jsonVariant["error"].toString();
            }

            emit tokenReplyReceived();
        });

        QEventLoop loop;
        connect(this, SIGNAL(tokenReplyReceived()), &loop, SLOT(quit()));
        loop.exec();

        return !accessToken_.isEmpty() && savedToken != accessToken_;
    }

    bool SpotifyWebAPI::testConnection() {
        return refreshToken();
    }

    bool SpotifyWebAPI::expired() {
        return QDateTime::currentDateTime() > expirationTime_;
    }

    QVector<Track> SpotifyWebAPI::searchTrack(const QString& query, const QString& limit) {
        auto *url = new QUrl(SEARCH_URL.arg(query, "track", limit));
        QNetworkRequest request(*url);

        auto header = QString("Bearer ").append(accessToken_);
        request.setRawHeader(QByteArray("Authorization"), header.toUtf8());
        request.setRawHeader(QByteArray("Accept"), "application/json");
        request.setHeader(QNetworkRequest::ContentTypeHeader, QVariant(QString("application/json")));

        QNetworkReply *reply = manager->get(request);

        connect(reply, &QNetworkReply::finished, [this, reply]() {

            QString answer = reply->readAll();
            QJsonObject jsonObject = answerToJson_(answer);

            itemResults_ = jsonObject["tracks"].toObject()["items"].toArray();

            emit searchReplyReceived();
        });

        QEventLoop loop;
        connect(this, SIGNAL(searchReplyReceived()), &loop, SLOT(quit()));
        loop.exec();

        auto results = new QVector<Track>();

        for (auto item : itemResults_) {
            auto trackData = item.toObject();
            auto artists = trackData["artists"].toArray();
            qDebug() << item;

            Track track;

            QString artistsText = "";
            int counter = 0;
            for (auto artist : artists) {
                if (counter > 0) {
                    artistsText.append(", ");
                }
                artistsText.append(artist.toObject()["name"].toString());
                counter++;
            }

            track.id = trackData["id"].toString();
            track.name = trackData["name"].toString();
            track.artists = artistsText;
            track.albumId = trackData["album"].toObject()["id"].toString();
            track.albumName = trackData["album"].toObject()["name"].toString();
            track.uri = trackData["uri"].toString();
            track.imageUrl = trackData["album"].toObject()["images"].toArray()[2].toObject()["url"].toString();
            track.isExplicit = trackData["explicit"].toBool();

            results->append(track);
        }

        return *results;
    }

    void SpotifyWebAPI::downloadImage(const QString& imageUrl, const QString& imageFilePath) {
        QNetworkRequest request(imageUrl);
        QNetworkReply *reply = manager->get(request);

        connect(reply, &QNetworkReply::finished, [this, reply, imageFilePath]() {
            QSaveFile file(imageFilePath);
            file.open(QIODevice::WriteOnly);
            file.write(reply->readAll());
            file.commit();
            emit imageReceived();
        });

        QEventLoop loop;
        connect(this, SIGNAL(imageReceived()), &loop, SLOT(quit()));
        loop.exec();
    }

    void SpotifyWebAPI::addItemToQueue(const QString& uri) {
        auto *url = new QUrl(ADD_ITEM_URL.arg(uri));
        QNetworkRequest request(*url);

        auto header = QString("Bearer ").append(accessToken_);
        request.setRawHeader(QByteArray("Authorization"), header.toUtf8());
        request.setRawHeader(QByteArray("Accept"), "application/json");
        request.setHeader(QNetworkRequest::ContentTypeHeader, QVariant(QString("application/json")));

        QByteArray postData;

        QNetworkReply *reply = manager->post(request, postData);

        connect(reply, &QNetworkReply::finished, [this]() {
            emit addedToQueue();
        });

        QEventLoop loop;
        connect(this, SIGNAL(addedToQueue()), &loop, SLOT(quit()));
        loop.exec();
    }

    void SpotifyWebAPI::skipToNextTrack() {
        auto *url = new QUrl(NEXT_TRACK_URL);
        QNetworkRequest request(*url);

        auto header = QString("Bearer ").append(accessToken_);
        request.setRawHeader(QByteArray("Authorization"), header.toUtf8());
        request.setRawHeader(QByteArray("Accept"), "application/json");
        request.setHeader(QNetworkRequest::ContentTypeHeader, QVariant(QString("application/json")));

        QByteArray postData;

        QNetworkReply *reply = manager->post(request, postData);

        connect(reply, &QNetworkReply::finished, [this]() {
            emit skippedTrack();
        });

        QEventLoop loop;
        connect(this, SIGNAL(skippedTrack()), &loop, SLOT(quit()));
        loop.exec();
    }

    void SpotifyWebAPI::play(const QString& uri, QString device) {
        if (device.isEmpty()) {
            device = activeDevice->id;
        }
        auto *url = new QUrl(PLAY_URL.arg(device));
        QNetworkRequest request(*url);

        auto header = QString("Bearer ").append(accessToken_);
        request.setRawHeader(QByteArray("Authorization"), header.toUtf8());
        request.setRawHeader(QByteArray("Accept"), "application/json");
        request.setHeader(QNetworkRequest::ContentTypeHeader, QVariant(QString("application/json")));

        QByteArray postData = QString(R"({"uris": ["%1"]})").arg(uri).toLocal8Bit();

        QNetworkReply *reply = manager->put(request, postData);

        connect(reply, &QNetworkReply::finished, [this, reply]() {
            qDebug() << "Reply:";
            qDebug() << reply->readAll();
            emit played();
        });

        QEventLoop loop;
        connect(this, SIGNAL(played()), &loop, SLOT(quit()));
        loop.exec();
    }

    QVector<Device> SpotifyWebAPI::getDevices() {
        auto *url = new QUrl(DEVICES_URL);
        QNetworkRequest request(*url);

        auto header = QString("Bearer ").append(accessToken_);
        request.setRawHeader(QByteArray("Authorization"), header.toUtf8());
        request.setRawHeader(QByteArray("Accept"), "application/json");
        request.setHeader(QNetworkRequest::ContentTypeHeader, QVariant(QString("application/json")));

        QNetworkReply *reply = manager->get(request);

        connect(reply, &QNetworkReply::finished, [this, reply]() {

            QString answer = reply->readAll();
            QJsonObject jsonObject = answerToJson_(answer);

            devicesResult_ = jsonObject["devices"].toArray();

            emit searchReplyReceived();
        });

        QEventLoop loop;
        connect(this, SIGNAL(searchReplyReceived()), &loop, SLOT(quit()));
        loop.exec();

        QVector<Device> result;

        activeDevice = nullptr;

        for (auto item : devicesResult_) {
            auto deviceData = item.toObject();

            auto *device = new Device();

            device->id = deviceData["id"].toString();
            device->name = deviceData["name"].toString();
            device->type = deviceData["type"].toString();
            device->isActive = deviceData["is_active"].toBool();

            if (device->isActive) {
                activeDevice = device;
            }

            result.append(*device);
        }

        return result;
    }
}