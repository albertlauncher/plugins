// Copyright (c) 2022-2023 Manuel Schneider


#include "albert/albert.h"
#include "albert/extension/queryhandler/standarditem.h"
#include "albert/logging.h"
#include "configwidget.h"
#include "plugin.h"
#include <QDesktopServices>
#include <QDir>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QUrl>
#include <array>
#include <vector>
ALBERT_LOGGING_CATEGORY("websearch")
using namespace albert;
using namespace std;

const char * ENGINES_FILE_NAME = "engines.json";

static QByteArray serializeEngines(const vector<SearchEngine> &engines)
{
    QJsonArray array;
    for (const SearchEngine& engine : engines) {
        QJsonObject object;
        object["id"] = engine.id;
        object["name"] = engine.name;
        object["url"] = engine.url;
        object["trigger"] = engine.trigger;
        object["iconPath"] = engine.iconUrl;
        array.append(object);
    }
    return QJsonDocument(array).toJson();
}

static vector<SearchEngine> deserializeEngines(const QByteArray &json)
{
    vector<SearchEngine> searchEngines;
    auto array = QJsonDocument::fromJson(json).array();
    for (const auto &value : array) {
        QJsonObject object = value.toObject();
        SearchEngine searchEngine;

        // Todo remove this in future releasea
        if (object.contains("id")){
            searchEngine.id = object["id"].toString();
        } else if (object.contains("guid")){
            searchEngine.id = object["guid"].toString();
        }

        if (searchEngine.id.isEmpty())
            searchEngine.id = QUuid::createUuid().toString(QUuid::WithoutBraces).left(8);

        searchEngine.name = object["name"].toString();
        searchEngine.trigger = object["trigger"].toString().trimmed();
        searchEngine.iconUrl = object["iconPath"].toString();
        searchEngine.url = object["url"].toString();
        searchEngines.push_back(searchEngine);
    }
    return searchEngines;
}

Plugin::Plugin()
{
    QFile file(configDir().filePath(ENGINES_FILE_NAME));
    if (file.open(QIODevice::ReadOnly))
        setEngines(deserializeEngines(file.readAll()));
    else
        restoreDefaultEngines();
}

const vector<SearchEngine> &Plugin::engines() const { return searchEngines_; }

void Plugin::setEngines(vector<SearchEngine> engines)
{
    sort(begin(engines), end(engines),
         [](auto a, auto b){ return a.name < b.name; });

    searchEngines_ = ::move(engines);

    if (QFile file(configDir().filePath(ENGINES_FILE_NAME));
        file.open(QIODevice::WriteOnly))
        file.write(serializeEngines(searchEngines_));
    else
        CRIT << QString("Could not write to file: '%1'.").arg(file.fileName());

    emit enginesChanged(searchEngines_);
}

void Plugin::restoreDefaultEngines()
{
    vector<SearchEngine> searchEngines;
    QFile file(QString(":%1").arg(ENGINES_FILE_NAME));
    if (file.open(QIODevice::ReadOnly)){
        auto array = QJsonDocument::fromJson(file.readAll()).array();
        for (const auto &value : array) {
            QJsonObject object = value.toObject();
            SearchEngine searchEngine;
            searchEngine.id = QUuid::createUuid().toString(QUuid::WithoutBraces).left(8);
            searchEngine.name = object["name"].toString();
            searchEngine.trigger = object["trigger"].toString();
            searchEngine.iconUrl = object["iconPath"].toString();
            searchEngine.url = object["url"].toString();
            searchEngines.push_back(searchEngine);
        }
    } else
        CRIT << "Failed reading default engines.";
    setEngines(searchEngines);
}

static shared_ptr<StandardItem> buildItem(const SearchEngine &se, const QString &search_term)
{
    QString url = QString(se.url).replace("%s", QUrl::toPercentEncoding(search_term));
    return StandardItem::make(
        se.id,
        se.name,
        Plugin::tr("Search %1 for '%2'").arg(se.name, search_term),
        QString("%1 %2").arg(se.name, search_term),
        {se.iconUrl},
        {{"run", Plugin::tr("Run websearch"), [url](){ openUrl(url); }}}
    );
}

vector<RankItem> Plugin::handleGlobalQuery(const GlobalQuery *query) const
{
    vector<RankItem> results;
    if (!query->string().isEmpty())
        for (const SearchEngine &se: searchEngines_)
            for (const auto &keyword : {QStringLiteral("%1 ").arg(se.trigger.toLower()),
                                        QStringLiteral("%1 ").arg(se.name.toLower())})
                if (auto prefix = query->string().toLower().left(keyword.size()); keyword.startsWith(prefix)){
                    results.emplace_back(buildItem(se, query->string().mid(prefix.size())),
                                         (float)prefix.length()/keyword.size());
                    break;  // max one of these icons, assumption: tigger is shorter and yields higer scores

                }
    return results;
}

vector<shared_ptr<Item>> Plugin::fallbacks(const QString &query) const
{
    vector<shared_ptr<Item>> results;
    if (!query.isEmpty())
        for (const SearchEngine &se: searchEngines_)
            results.emplace_back(buildItem(se, query.isEmpty()?"…":query));
    return results;
}

QWidget *Plugin::buildConfigWidget() { return new ConfigWidget(const_cast<Plugin*>(this)); }
