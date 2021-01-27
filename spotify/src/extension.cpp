// Copyright (C) 2014-2021 Manuel Schneider, Ivo Šmerek

#include <QPointer>
#include <stdexcept>
#include <QMessageBox>
#include <QtCore/QJsonDocument>
#include <QtCore/QJsonObject>
#include <QtNetwork/QNetworkReply>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPushButton>
#include <albert/util/standardactions.h>
#include "albert/util/standarditem.h"
#include "configwidget.h"
#include "extension.h"
#include "spotifyWebAPI.h"
#include "xdg/iconlookup.h"
Q_LOGGING_CATEGORY(qlc, "spotify")
#define DEBG qCDebug(qlc,).noquote()
#define INFO qCInfo(qlc,).noquote()
#define WARN qCWarning(qlc,).noquote()
#define CRIT qCCritical(qlc,).noquote()
using namespace Core;
using namespace std;

namespace {

}

class Spotify::Private
{
public:
    QPointer<ConfigWidget> widget;
    QString clientId;
    QString clientSecret;
    QString refreshToken;
    QString spotifyExecutable;
    bool explicitState = true;
    int numberOfResults = 5;
    SpotifyWebAPI *api = nullptr;
};


/** ***************************************************************************/
Spotify::Extension::Extension()
    : Core::Extension("org.albert.extension.spotify"), // Must match the id in metadata
      Core::QueryHandler(Core::Plugin::id()),
      d(new Private) {

    registerQueryHandler(this);

    d->api = new SpotifyWebAPI(this);

    d->clientId = settings().value("client_id").toString();
    d->clientSecret = settings().value("client_secret").toString();
    d->refreshToken = settings().value("refresh_token").toString();
    d->explicitState = settings().value("explicit_state").toBool();
    d->numberOfResults = settings().value("number_or_results").toInt();
    d->spotifyExecutable = settings().value("spotify_executable").toString();

    if (d->numberOfResults == 0) {
        d->numberOfResults = 5;
    }

    if (d->spotifyExecutable.isEmpty()) {
        d->spotifyExecutable = "spotify";
    }
}



/** ***************************************************************************/
Spotify::Extension::~Extension() = default;



/** ***************************************************************************/
QWidget *Spotify::Extension::widget(QWidget *parent) {
    if (d->widget.isNull()) {
        d->widget = new ConfigWidget(parent);
    }

    // Initialize the content and connect the signals

    d->widget->ui.lineEdit_client_id->setText(d->clientId);
    connect(d->widget->ui.lineEdit_client_id, &QLineEdit::textEdited, [this](const QString &s){
        d->clientId = s;
        settings().setValue("client_id", s);
    });

    d->widget->ui.lineEdit_client_secret->setText(d->clientSecret);
    connect(d->widget->ui.lineEdit_client_secret, &QLineEdit::textEdited, [this](const QString &s){
        d->clientSecret = s;
        settings().setValue("client_secret", s);
    });

    d->widget->ui.lineEdit_refresh_token->setText(d->refreshToken);
    connect(d->widget->ui.lineEdit_refresh_token, &QLineEdit::textEdited, [this](const QString &s){
        d->refreshToken = s;
        settings().setValue("refresh_token", s);
    });

    d->widget->ui.checkBox_explicit->setCheckState(d->explicitState ? Qt::CheckState::Checked : Qt::CheckState::Unchecked);
    connect(d->widget->ui.checkBox_explicit, &QCheckBox::stateChanged, [this](const int s){
        d->explicitState = s;
        settings().setValue("explicit_state", s);
    });

    d->widget->ui.spinBox_number_of_results->setValue(d->numberOfResults);
    connect(d->widget->ui.spinBox_number_of_results, &QSpinBox::textChanged, [this](const QString &s){
        d->numberOfResults = s.toInt();
        settings().setValue("number_or_results", s);
    });

    d->widget->ui.lineEdit_spotify_executable->setText(d->spotifyExecutable);
    connect(d->widget->ui.lineEdit_spotify_executable, &QLineEdit::textEdited, [this](const QString &s){
        d->spotifyExecutable = s;
        settings().setValue("spotify_executable", s);
    });

    // Bind "Test connection" button

    connect(d->widget->ui.pushButton_test_connection, &QPushButton::clicked, [this](){
        d->api->setConnection(d->clientId, d->clientSecret, d->refreshToken);
        d->api->setQNetworkAccessManager(new QNetworkAccessManager());

        bool status = d->api->testConnection();

        QString message = "Everything is set up correctly.";
        if (!status) {
            message = QString("Spotify Web API returns: \"%1\"\nPlease, check all input fields.")
                    .arg(d->api->lastErrorMessage);
            if (d->api->lastErrorMessage.isEmpty()) {
                message = "Can't get an answer from the server.\nPlease, check your internet connection.";
            }
        }

        auto messageBox = new QMessageBox();
        messageBox->setWindowTitle(status ? "Success" : "API error");
        messageBox->setText(message);
        messageBox->setIcon(status ? QMessageBox::Information : QMessageBox::Critical);
        messageBox->exec();
    });

    return d->widget;
}



/** ***************************************************************************/
void Spotify::Extension::setupSession() {
    d->api->setConnection(d->clientId, d->clientSecret, d->refreshToken);

    if(!QDir(COVERS_DIR_PATH).exists()) {
        QDir().mkdir(COVERS_DIR_PATH);
    }
}



/** ***************************************************************************/
void Spotify::Extension::teardownSession() {

}



/** ***************************************************************************/
void Spotify::Extension::handleQuery(Core::Query * query) const {
    if (query->string().trimmed().isEmpty())
        return;

    d->api->setQNetworkAccessManager(new QNetworkAccessManager());

    // If there is no internet connection, make one alerting item to let the user know.
    if (!d->api->testInternetConnection()) {
        DEBG << "No internet connection!";

        auto result = makeStdItem("no-internet", "", "Can't get an answer from the server.");
        result->setSubtext("Please, check your internet connection.");

        query->addMatch(move(result), UINT_MAX);
        return;
    }

    // If the access token expires, try to refresh it or alert the user what is wrong.
    if (d->api->expired()) {
        DEBG << "Token expired. Refreshing";
        if (!d->api->refreshToken()) {
            auto result = makeStdItem("wrong-credentials", "", "Wrong credentials");
            result->setSubtext(d->api->lastErrorMessage + ". Please, check extension settings.");

            query->addMatch(move(result), UINT_MAX);
            return;
        }
    }

    // Search for tracks on Spotify using the query.
    auto results = d->api->searchTracks(query->string(), d->numberOfResults);

    // Get available Spotify devices.
    auto *devices = d->api->getDevices();

    for (const auto& track : results) {
        // Deal with explicit tracks according to user setting.
        if (track.isExplicit && !d->explicitState) {
            continue;
        }

        auto filename = QString("%1/%2.jpeg").arg(COVERS_DIR_PATH, track.albumId);

        // Download cover image of the album.
        d->api->downloadImage(track.imageUrl, filename);

        // Create a standard item with a track name in title and album with artists in subtext.
        auto result = makeStdItem(track.id, filename, track.name);
        result->setSubtext(QString("%1 (%2)").arg(track.albumName, track.artists));

        // First default action with intelligent device chooser.
        auto playTrack = makeFuncAction("Play this track on Spotify", [this, track, devices]()
        {
            // Check if the last-used device is still available.
            bool lastDeviceConfirmed = false;
            QString lastDevice = settings().value("last_device").toString();
            if (!lastDevice.isEmpty() || !devices->isEmpty()) {
                for (const auto& device : *devices) {
                    if (device.id == lastDevice) {
                        lastDeviceConfirmed = true;
                        break;
                    }
                }
            }

            if (d->api->activeDevice) {
                // If available, use an active device and play the track.
                // TODO: Maybe let user choose in setting if prefer active or last-used device.
                d->api->play(track.uri, d->api->activeDevice->id);
                settings().setValue("last_device", d->api->activeDevice->id);
            } else if (lastDeviceConfirmed) {
                // If there is not an active device, use last-used one.
                d->api->play(track.uri, lastDevice);
            } else if (!devices->isEmpty()) {
                // Use the first available device.
                d->api->play(track.uri, devices[0][0].id);
                settings().setValue("last_device", devices[0][0].id);
            } else {
                // Run local Spotify client, wait until it loads, and play the track.
                makeProcAction("Run Spotify", QStringList() << d->spotifyExecutable)->activate();
                d->api->waitForDeviceAndPlay(track.uri, 10);
            }
        });

        // Action to add track to the Spotify queue.
        auto addToQueue = makeFuncAction("Add to the Spotify queue", [this, track]()
        {
            d->api->addItemToQueue(track.uri);
        });

        result->addAction(playTrack);
        result->addAction(addToQueue);

        // For each device except active create action to transfer Spotify playback to this device.
        for (const auto& device : *devices) {
            if (device.isActive) continue;

            auto action = makeFuncAction(QString("Play on %1 (%2)").arg(device.type, device.name), [this, track, device]()
            {
                d->api->play(track.uri, device.id);
                settings().setValue("last_device", device.id);
            });

            result->addAction(action);
        }

        query->addMatch(move(result), UINT_MAX);
    }
}

QueryHandler::ExecutionType Spotify::Extension::executionType() const {
    return QueryHandler::ExecutionType::Realtime;
}