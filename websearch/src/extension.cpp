// Copyright (C) 2014-2020 Manuel Schneider

#include <QDesktopServices>
#include <QDir>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QPointer>
#include <QRegularExpression>
#include <QStandardPaths>
#include <QUrl>
#include <array>
#include <map>
#include <string>
#include <vector>
#include "albert/util/standardactions.h"
#include "albert/util/standarditem.h"
#include "configwidget.h"
#include "enginesmodel.h"
#include "extension.h"
#include "xdg/iconlookup.h"
Q_LOGGING_CATEGORY(qlc, "websearch")
#define DEBG qCDebug(qlc,).noquote()
#define INFO qCInfo(qlc,).noquote()
#define WARN qCWarning(qlc,).noquote()
#define CRIT qCCritical(qlc,).noquote()
using namespace Core;
using namespace std;


namespace {

vector<Websearch::SearchEngine> defaultSearchEngines = {
    {"Google",        "gg ",  ":google",    "https://www.google.com/search?q=%s"},
    {"Youtube",       "yt ",  ":youtube",   "https://www.youtube.com/results?search_query=%s"},
    {"Amazon",        "ama ", ":amazon",    "http://www.amazon.com/s/?field-keywords=%s"},
    {"Ebay",          "eb ",  ":ebay",      "http://www.ebay.com/sch/i.html?_nkw=%s"},
    {"GitHub",        "gh ",  ":github",    "https://github.com/search?utf8=✓&q=%s"},
    {"Wolfram Alpha", "=",    ":wolfram",   "https://www.wolframalpha.com/input/?i=%s"},
    {"DuckDuckGo",    "dd ",  ":duckduckgo","https://duckduckgo.com/?q=%s"},
};

shared_ptr<Core::Item> buildWebsearchItem(const Websearch::SearchEngine &se, const QString &searchterm) {

    QString urlString = QString(se.url).replace("%s", QUrl::toPercentEncoding(searchterm));
    QUrl url = QUrl(urlString);
    QString desc = QString("Start %1 search in your browser").arg(se.name);

    return makeStdItem(se.name,
                       se.iconPath,
                       se.name,
                       desc,
                       ActionList{ makeUrlAction("Open URL", url) },
                       QString("%1%2").arg(se.trigger, searchterm));
}

static constexpr const char * ENGINES_FILE_NAME = "engines.json";

}



/** ***************************************************************************/
/** ***************************************************************************/
/** ***************************************************************************/
/** ***************************************************************************/
class Websearch::Private
{
public:
    QPointer<ConfigWidget> widget;
    vector<SearchEngine> searchEngines;
};



/** ***************************************************************************/
/** ***************************************************************************/
/** ***************************************************************************/
/** ***************************************************************************/
Websearch::Extension::Extension()
    : Core::Extension("org.albert.extension.websearch"),
      Core::QueryHandler(Core::Plugin::id()),
      d(new Private) {

    registerQueryHandler(this);
    registerFallbackProvider(this);

    // Move config file from old location to new. (data -> config) TODO: REMOVE in 0.14
    QString oldpath = QDir(QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation))
            .filePath(QString("%1.json").arg(Core::Plugin::id()));
    QString enginesJson = configLocation().filePath(ENGINES_FILE_NAME);
    if ( QFile::exists(oldpath) ) {
        if ( QFile::exists(enginesJson) )
            QFile::remove(oldpath);
        else
            QFile::rename(oldpath, enginesJson);
    }

    // Deserialize engines
    QFile file(enginesJson);
    if (file.open(QIODevice::ReadOnly)) {
        QJsonArray array = QJsonDocument::fromJson(file.readAll()).array();
        SearchEngine searchEngine;
        for ( const QJsonValue& value : array) {
            QJsonObject object = value.toObject();
            searchEngine.name     = object["name"].toString();
            searchEngine.trigger  = object["trigger"].toString();
            searchEngine.iconPath = object["iconPath"].toString();
            searchEngine.url      = object["url"].toString();
            d->searchEngines.push_back(searchEngine);
        }
    } else {
        WARN << qPrintable(QString("Could not load from file: '%1'.").arg(enginesJson));
        setEngines(defaultSearchEngines);
    }
}



/** ***************************************************************************/
Websearch::Extension::~Extension() {

}



/** ***************************************************************************/
QWidget *Websearch::Extension::widget(QWidget *parent) {
    if (d->widget.isNull())
        d->widget = new ConfigWidget(this, parent);
    return d->widget;
}



/** ***************************************************************************/
QStringList Websearch::Extension::triggers() const {
      QStringList triggers;
      for ( const SearchEngine& se : d->searchEngines )
          triggers.push_back(se.trigger);
      return triggers;
}



/** ***************************************************************************/
void Websearch::Extension::handleQuery(Core::Query * query) const {

    if ( query->isTriggered() ) {
        for ( const SearchEngine &se : d->searchEngines )
            if ( query->trigger() == se.trigger )
                query->addMatch(buildWebsearchItem(se, query->string()));
    }
    else
    {
        QUrl url = QUrl::fromUserInput(query->string().trimmed());

        // Check syntax and TLD validity
        if ( url.isValid() && ( query->string().startsWith("http://") ||  // explict scheme
                                query->string().startsWith("https://") ||  // explict scheme
                                ( QRegularExpression(R"R(\S+\.\S+$)R").match(url.host()).hasMatch() &&
                                  !url.topLevelDomain().isNull()) ) ) {  // valid TLD

            QString icon = XDG::IconLookup::iconPath({"www", "web-browser", "emblem-web"});

            query->addMatch(makeStdItem("valid_url",
                                        icon.isNull() ? ":favicon" : icon,
                                        "Open url in browser",
                                        QString("Visit %1").arg(url.authority()),
                                        ActionList{ makeUrlAction("Open URL", url) }
                                        ), UINT_MAX);
        }
    }
}



/** ***************************************************************************/
vector<shared_ptr<Core::Item>> Websearch::Extension::fallbacks(const QString & searchterm) {
    vector<shared_ptr<Core::Item>> res;
    for (const SearchEngine &se : d->searchEngines)
        res.push_back(buildWebsearchItem(se, searchterm));
    return res;
}



/** ***************************************************************************/
const vector<Websearch::SearchEngine> &Websearch::Extension::engines() const {
    return d->searchEngines;
}



/** ***************************************************************************/
void Websearch::Extension::setEngines(const vector<Websearch::SearchEngine> &engines) {
    d->searchEngines = engines;
    emit enginesChanged(d->searchEngines);

    // Serialize the engines
    QFile file(configLocation().filePath(ENGINES_FILE_NAME));
    if (file.open(QIODevice::WriteOnly)) {
        QJsonArray array;
        for ( const SearchEngine& searchEngine : d->searchEngines ) {
            QJsonObject object;
            object["name"]     = searchEngine.name;
            object["url"]      = searchEngine.url;
            object["trigger"]  = searchEngine.trigger;
            object["iconPath"] = searchEngine.iconPath;
            array.append(object);
        }
        file.write(QJsonDocument(array).toJson());
    } else
        CRIT << qPrintable(QString("Could not write to file: '%1'.").arg(file.fileName()));
}



/** ***************************************************************************/
void Websearch::Extension::restoreDefaultEngines() {
    setEngines(defaultSearchEngines);
}
