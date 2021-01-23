// Copyright (C) 2014-2021 Manuel Schneider, Ivo Šmerek

#include <QPointer>
#include <QSettings>
#include <stdexcept>
#include <QMessageBox>
#include <QtCore/QJsonArray>
#include <QtCore/QJsonDocument>
#include <QtCore/QJsonObject>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkReply>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPushButton>
#include <QtCore/QEventLoop>
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
    SpotifyWebAPI *api = new SpotifyWebAPI();
};


/** ***************************************************************************/
Spotify::Extension::Extension()
    : Core::Extension("org.albert.extension.spotify"), // Must match the id in metadata
      Core::QueryHandler(Core::Plugin::id()),
      d(new Private) {

    registerQueryHandler(this);

    d->clientId = settings().value("client_id").toString();
    d->clientSecret = settings().value("client_secret").toString();
    d->refreshToken = settings().value("refresh_token").toString();


    // You can throw in the constructor if something fatal happened
    // throw std::runtime_error( "Description of error." );
    // throw std::string( "Description of error." );
    // throw QString( "Description of error." );
    // throw "Description of error.";
    // throw; // Whatever prints "unknown error"
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

    // Bind "Test connection" button

    connect(d->widget->ui.pushButton_test_connection, &QPushButton::clicked, [this](){
        d->api->setConnection(d->clientId, d->clientSecret, d->refreshToken);

        bool status = d->api->testConnection();

        QString message = "Everything is set up correctly.";
        if (!status) {
            message = QString("Spotify Web API returns: \"%1\"\nPlease check all input fields.")
                    .arg(d->api->lastErrorMessage);
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

    if(!QDir("/tmp/albert-spotify").exists()) {
        QDir().mkdir("/tmp/albert-spotify-covers");
    }
}



/** ***************************************************************************/
void Spotify::Extension::teardownSession() {

}



/** ***************************************************************************/
void Spotify::Extension::handleQuery(Core::Query * query) const {

    /*
     * Things change so often I wont maintain this tutorial here. Check the relevant headers.
     *
     * - core/extension.h
     * - core/queryhandler.h
     * - core/query.h
     * - core/item.h
     * - core/action.h
     * - util/standarditem.h
     * - util/offlineindex.h
     * - util/standardindexitem.h
     *
     * Use
     *
     *   query->addMatch(my_item)
     *
     * to add matches. If you created a throw away item MOVE it instead of
     * copying e.g.:
     *
     *   query->addMatch(std::move(my_tmp_item))
     *
     * The relevance factor is optional. (Defaults to 0) its a usigned integer depicting the
     * relevance of the item 0 mean not relevant UINT_MAX is totally relevant (exact match).
     * E.g. it the query is "it" and your items name is "item"
     *
     *   my_item.name().startswith(query->string)
     *
     * is a naive match criterion and
     *
     *   UINT_MAX / ( query.searchterm().size() / my_item.name().size() )
     *
     * a naive match factor.
     *
     * If you have a lot of items use the iterator versions addMatches, e.g. like that
     *
     *   query->addMatches(my_items.begin(), my_items.end());
     *
     * If the items in the container are temporary object move them to avoid uneccesary
     * reference counting:
     *
     *   query->addMatches(std::make_move_iterator(my_tmp_items.begin()),
     *                     std::make_move_iterator(my_tmp_items.end()));
     */

    if (query->string().trimmed().isEmpty())
        return;

    if (d->api->expired()) {
        DEBG << "Token expired. Refreshing";
        d->api->refreshToken();
    }

    auto results = d->api->searchTrack(query->string(), "5");
    auto devices = d->api->getDevices();

    for (const auto& track : results) {
        auto filename = QString("%1/%2.jpeg").arg(COVERS_DIR_PATH, track.albumId);

        d->api->downloadImage(track.imageUrl, filename);

        auto result = makeStdItem(
                track.id,
                filename,
                QString("%1").arg(track.name),
                QString("%1 (%2)").arg(track.albumName, track.artists),
                ActionList { },
                "none",
                Item::Urgency::Alert
        );

        auto playTrack = makeFuncAction("Play on active Spotify device", [this, track]()
        {
            d->api->play(track.uri);
        });

        auto addToQueue = makeFuncAction("Add to the Spotify queue", [this, track]()
        {
            d->api->addItemToQueue(track.uri);
        });

        result->addAction(playTrack);
        result->addAction(addToQueue);

        for (auto device : devices) {
            if (device.isActive) continue;

            auto action = makeFuncAction(QString("Play on %1 (%2)").arg(device.type, device.name), [this, track, device]()
            {
                d->api->play(track.uri, device.id);
            });

            result->addAction(action);
        }

        query->addMatch(result, UINT_MAX);
    }
}