// albert - a simple application launcher for linux
// Copyright (C) 2014-2017 Manuel Schneider
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#include <QDebug>
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
#include <vector>
#include <string>
#include "configwidget.h"
#include "enginesmodel.h"
#include "extension.h"
#include "util/standarditem.h"
#include "xdg/iconlookup.h"
using std::shared_ptr;
using std::vector;
using namespace Core;


namespace {

std::vector<Websearch::SearchEngine> defaultSearchEngines = {
    {"Google",        "gg ",  ":google",    "https://www.google.com/search?q=%s"},
    {"Youtube",       "yt ",  ":youtube",   "https://www.youtube.com/results?search_query=%s"},
    {"Amazon",        "ama ", ":amazon",    "http://www.amazon.com/s/?field-keywords=%s"},
    {"Ebay",          "eb ",  ":ebay",      "http://www.ebay.com/sch/i.html?_nkw=%s"},
    {"GitHub",        "gh ",  ":github",    "https://github.com/search?utf8=✓&q=%s"},
    {"Wikipedia",     "wiki ",":wikipedia", "https://en.wikipedia.org/w/index.php?search=%s"},
    {"Wolfram Alpha", "=",    ":wolfram",   "https://www.wolframalpha.com/input/?i=%s"},
    {"DuckDuckGo",    "dd ",  ":duckduckgo","https://duckduckgo.com/?q=%s"},
};

shared_ptr<Core::Item> buildWebsearchItem(const Websearch::SearchEngine &se, const QString &searchterm) {

    QString urlString = QString(se.url).replace("%s", QUrl::toPercentEncoding(searchterm));
    QUrl url = QUrl(urlString);
    QString desc = QString("Start %1 search in your browser").arg(se.name);

    auto item = std::make_shared<StandardItem>(se.name);
    item->setText(se.name);
    item->setSubtext(desc);
    item->setIconPath(se.iconPath);
    item->setCompletionString(QString("%1%2").arg(se.trigger, searchterm));
    item->emplaceAction(desc, [=](){ QDesktopServices::openUrl(url); });

    return item;
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
    std::vector<SearchEngine> searchEngines;
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
        qWarning() << qPrintable(QString("Could not load from file: '%1'.").arg(enginesJson));
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
            auto item = std::make_shared<StandardItem>("valid_url");
            item->setText(QString("Open url in browser"));
            item->setSubtext(QString("Visit %1").arg(url.authority()));
            item->setCompletionString(query->string());
            QString icon = XDG::IconLookup::iconPath({"www", "web-browser", "emblem-web"});
            item->setIconPath(icon.isNull() ? ":favicon" : icon);
            item->emplaceAction("Open URL", [url](){ QDesktopServices::openUrl(url); });

            query->addMatch(std::move(item), UINT_MAX);
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
const std::vector<Websearch::SearchEngine> &Websearch::Extension::engines() const {
    return d->searchEngines;
}



/** ***************************************************************************/
void Websearch::Extension::setEngines(const std::vector<Websearch::SearchEngine> &engines) {
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
        qCritical() << qPrintable(QString("Could not write to file: '%1'.").arg(file.fileName()));
}



/** ***************************************************************************/
void Websearch::Extension::restoreDefaultEngines() {
    setEngines(defaultSearchEngines);
}
