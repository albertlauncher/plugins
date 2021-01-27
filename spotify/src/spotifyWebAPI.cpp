// Copyright (C) 2020-2021 Ivo Šmerek

#include <QtConcurrent/QtConcurrent>
#include <QtNetwork/QNetworkReply>

#include "spotifyWebAPI.h"

namespace Spotify {

    SpotifyWebAPI::SpotifyWebAPI(QObject* parent) {
        manager = new QNetworkAccessManager(this);
        setParent(parent);
    }

    SpotifyWebAPI::~SpotifyWebAPI() {
        delete manager;
    }

    QJsonObject SpotifyWebAPI::answerToJson_(const QString& answer) {
        QJsonDocument doc = QJsonDocument::fromJson(answer.toUtf8());
        QJsonObject jsonObject = doc.object();
        return jsonObject;
    }

    void SpotifyWebAPI::waitForSignal_(const QObject *sender, const char *signal) {
        QEventLoop loop;
        connect(sender, signal, &loop, SLOT(quit()));
        loop.exec();
    }

    QString SpotifyWebAPI::waitForDevice_(QString uri, int timeout) {
        int counter = 0;
        while (counter < timeout) {
            QThread::sleep(1);
            auto device = getFirstDeviceId();
            if (!device.isEmpty()) {
                emit deviceReady(std::move(uri), device);
                return device;
            }
            counter++;
        }

        return "";
    }

    QNetworkRequest SpotifyWebAPI::buildRequest_(const QUrl& url) {
        auto request = new QNetworkRequest(url);
        auto header = QString("Bearer ") + accessToken_;
        request->setRawHeader(QByteArray("Authorization"), header.toUtf8());
        request->setRawHeader(QByteArray("Accept"), "application/json");
        request->setHeader(QNetworkRequest::ContentTypeHeader, QVariant(QString("application/json")));
        return *request;
    }

    bool SpotifyWebAPI::testInternetConnection() {
        try {
            auto *url = new QUrl(TOKEN_URL);
            QNetworkRequest request(*url);

            if (manager->thread() != QThread::currentThread()) {
                return false;
            }
            QNetworkReply *reply = manager->get(request);

            waitForSignal_(reply, SIGNAL(finished()));
            return reply->bytesAvailable();
        } catch (...) {
            return false;
        }
    }

    void SpotifyWebAPI::setConnection(QString clientId, QString clientSecret, QString refreshToken) {
        clientId_ = std::move(clientId);
        clientSecret_ = std::move(clientSecret);
        refreshToken_ = std::move(refreshToken);
        expirationTime_ = QDateTime::currentDateTime();
    }

    bool SpotifyWebAPI::refreshToken() {
        auto url = QUrl(TOKEN_URL);
        QNetworkRequest request(url);

        auto hash = QString("%1:%2").arg(clientId_, clientSecret_).toUtf8().toBase64();
        auto header = QString("Basic ").append(hash);
        request.setRawHeader(QByteArray("Authorization"), header.toUtf8());
        request.setHeader(QNetworkRequest::ContentTypeHeader,
                          QVariant(QString("application/x-www-form-urlencoded")));

        QByteArray postData = QString("grant_type=refresh_token&refresh_token=%1").arg(refreshToken_).toLocal8Bit();

        QString savedToken = accessToken_;

        if (manager->thread() != QThread::currentThread()) {
            return false;
        }
        QNetworkReply *reply = manager->post(request, postData);

        connect(reply, &QNetworkReply::finished, [this, reply]() {
            QString answer = reply->readAll();
            QJsonObject jsonVariant = answerToJson_(answer);

            accessToken_ = "";

            if (!jsonVariant["access_token"].isUndefined()) {
                accessToken_ = jsonVariant["access_token"].toString();
                expirationTime_ = QDateTime::currentDateTime().addSecs(jsonVariant["expires_in"].toInt());
            }

            if (!jsonVariant["error_description"].isUndefined()) {
                lastErrorMessage = jsonVariant["error_description"].toString();
            } else {
                lastErrorMessage = jsonVariant["error"].toString();
            }
        });

        waitForSignal_(reply, SIGNAL(finished()));

        return !accessToken_.isEmpty() && savedToken != accessToken_;
    }

    bool SpotifyWebAPI::testConnection() {
        return refreshToken();
    }

    bool SpotifyWebAPI::expired() {
        return QDateTime::currentDateTime() > expirationTime_;
    }

    QVector<Track> SpotifyWebAPI::searchTracks(const QString& query, const int limit) {
        auto url = QUrl(SEARCH_URL.arg(query, "track", QString::number(limit)));
        QNetworkRequest request = buildRequest_(url);

        if (manager->thread() != QThread::currentThread()) {
            return QVector<Track>();
        }
        QNetworkReply *reply = manager->get(request);

        auto *itemResults = new QJsonArray();

        connect(reply, &QNetworkReply::finished, [reply, itemResults]() {

            QString answer = reply->readAll();
            QJsonObject jsonObject = answerToJson_(answer);

            *itemResults = jsonObject["tracks"].toObject()["items"].toArray();
        });

        waitForSignal_(reply, SIGNAL(finished()));

        auto results = new QVector<Track>();

        for (auto item : *itemResults) {
            auto trackData = item.toObject();
            auto artists = trackData["artists"].toArray();

            auto track = new Track();

            QString artistsText = "";
            int counter = 0;
            for (auto artist : artists) {
                if (counter > 0) {
                    artistsText.append(", ");
                }
                artistsText.append(artist.toObject()["name"].toString());
                counter++;
            }

            track->id = trackData["id"].toString();
            track->name = trackData["name"].toString();
            track->artists = artistsText;
            track->albumId = trackData["album"].toObject()["id"].toString();
            track->albumName = trackData["album"].toObject()["name"].toString();
            track->uri = trackData["uri"].toString();
            track->imageUrl = trackData["album"].toObject()["images"].toArray()[2].toObject()["url"].toString();
            track->isExplicit = trackData["explicit"].toBool();

            results->append(*track);
        }

        return *results;
    }

    void SpotifyWebAPI::downloadImage(const QString& imageUrl, const QString& imageFilePath) {
        fileLock_.lockForWrite();

        QFileInfo fileInfo(imageFilePath);
        if (fileInfo.exists()) {
            fileLock_.unlock();
            return;
        }

        if (manager->thread() != QThread::currentThread()) {
            fileLock_.unlock();
            return;
        }

        QNetworkRequest request(imageUrl);
        QNetworkReply *reply = manager->get(request);

        connect(reply, &QNetworkReply::finished, [reply, imageFilePath]() {
            if (reply->bytesAvailable()) {
                QSaveFile file(imageFilePath);
                file.open(QIODevice::WriteOnly);
                file.write(reply->readAll());
                file.commit();
            }
        });

        waitForSignal_(reply, SIGNAL(finished()));
        fileLock_.unlock();
    }

    void SpotifyWebAPI::addItemToQueue(const QString& uri) {
        manager = new QNetworkAccessManager();
        auto url = QUrl(ADD_ITEM_URL.arg(uri));
        QNetworkRequest request = buildRequest_(url);

        manager->post(request, "");
    }

    void SpotifyWebAPI::play(const QString& uri, QString device) {
        manager = new QNetworkAccessManager();
        if (device.isEmpty() && activeDevice) {
            device = activeDevice->id;
        }
        auto url = QUrl(PLAY_URL.arg(device));
        QNetworkRequest request = buildRequest_(url);

        QByteArray postData = QString(R"({"uris": ["%1"]})").arg(uri).toLocal8Bit();

        manager->put(request, postData);
    }

    QString SpotifyWebAPI::waitForDeviceAndPlay(const QString& uri, int timeout) {
        connect(this, SIGNAL(deviceReady(QString, QString)), this, SLOT(play(QString, QString)));

        QtConcurrent::run([=]() {
            manager = new QNetworkAccessManager();
            waitForDevice_(uri, timeout);
        });

        return "";
    }

    QVector<Device> *SpotifyWebAPI::getDevices() {
        auto url = QUrl(DEVICES_URL);
        QNetworkRequest request = buildRequest_(url);

        if (manager->thread() != QThread::currentThread()) {
            return new QVector<Device>();
        }
        QNetworkReply *reply = manager->get(request);

        auto *devicesResult = new QJsonArray();

        connect(reply, &QNetworkReply::finished, [reply, devicesResult]() {
            QString answer = reply->readAll();
            QJsonObject jsonObject = answerToJson_(answer);

            *devicesResult = jsonObject["devices"].toArray();
        });

        waitForSignal_(reply, SIGNAL(finished()));

        auto result = new QVector<Device>();

        activeDevice = nullptr;

        for (auto item : *devicesResult) {
            auto deviceData = item.toObject();

            auto *device = new Device();

            device->id = deviceData["id"].toString();
            device->name = deviceData["name"].toString();
            device->type = deviceData["type"].toString();
            device->isActive = deviceData["is_active"].toBool();

            if (device->isActive) {
                activeDevice = device;
            }

            result->append(*device);
        }

        return result;
    }

    QString SpotifyWebAPI::getFirstDeviceId() {
        QVector<Device> *devices_ = getDevices();
        if (devices_->isEmpty()) {
            return "";
        }
        return devices_[0][0].id;
    }
}