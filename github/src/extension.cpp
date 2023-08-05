// Copyright (C) 2014-2018 Manuel Schneider

#include <QDebug>
#include <QPointer>
#include <QtNetworkAuth>
#include <QDesktopServices>
#include <stdexcept>
#include "util/standarditem.h"
#include "xdg/iconlookup.h"
#include "configwidget.h"
#include "extension.h"
using namespace Core;
using namespace std;

class GitHub::Private
{
public:
    QPointer<ConfigWidget> widget;
    QOAuth2AuthorizationCodeFlow oauth2;
};


/** ***************************************************************************/
GitHub::Extension::Extension()
    : Core::Extension("org.albert.extension.github"), // Must match the id in metadata
      Core::QueryHandler(Core::Plugin::id()),
      d(new Private) {

    registerQueryHandler(this);

    auto replyHandler = new QOAuthHttpServerReplyHandler(1337, this);
    d->oauth2.setReplyHandler(replyHandler);
    d->oauth2.setAuthorizationUrl(QUrl("https://github.com/login/oauth/authorize"));
    d->oauth2.setClientIdentifier("manuelschneid3r");
    d->oauth2.setAccessTokenUrl(QUrl("https://github.com/login/oauth/access_token"));
    d->oauth2.setScope("identity read");
    d->oauth2.grant();


    d->oauth2.setModifyParametersFunction([&](QAbstractOAuth::Stage stage, QVariantMap *parameters) {
        if (stage == QAbstractOAuth::Stage::RequestingAuthorization)
            parameters->insert("duration", "permanent");
    });
    connect(&d->oauth2, &QOAuth2AuthorizationCodeFlow::authorizeWithBrowser,
            &QDesktopServices::openUrl);
    connect(&d->oauth2, &QOAuth2AuthorizationCodeFlow::authorizeWithBrowser,
            [ ]{ qCritical() << "authoirize pkls"; });


    connect(&d->oauth2, &QOAuth2AuthorizationCodeFlow::statusChanged, [=](
            QAbstractOAuth::Status status) {
        qCritical() << (int)status;
    });


    // You can throw in the constructor if something fatal happened
    // throw std::runtime_error( "Description of error." );
    // throw std::string( "Description of error." );
    // throw QString( "Description of error." );
    // throw "Description of error.";
    // throw; // Whatever prints "unknown error"
}



/** ***************************************************************************/
GitHub::Extension::~Extension() {

}



/** ***************************************************************************/
QWidget *GitHub::Extension::widget(QWidget *parent) {
    if (d->widget.isNull()) {
        d->widget = new ConfigWidget(parent);
    }
    return d->widget;
}



/** ***************************************************************************/
void GitHub::Extension::setupSession() {

}



/** ***************************************************************************/
void GitHub::Extension::teardownSession() {

}



/** ***************************************************************************/
void GitHub::Extension::handleQuery(Core::Query *) const {

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
}

