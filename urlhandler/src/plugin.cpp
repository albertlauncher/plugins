// Copyright (c) 2022 Manuel Schneider

#include "albert/extension/queryhandler/standarditem.h"
#include "plugin.h"
#include <QDesktopServices>
#include <QFile>
#include <QTextStream>
#include <QUrl>
using namespace albert;
using namespace std;

Plugin::Plugin()
{
    QFile file(":tlds");
    if (!file.open(QIODevice::ReadOnly))
        throw runtime_error("Unable to read tld resource");
    valid_tlds << QTextStream(&file).readAll().split("\n");
    std::sort(valid_tlds.begin(), valid_tlds.end());
}

vector<RankItem> Plugin::handleGlobalQuery(const GlobalQuery *query) const
{
    vector<RankItem> results;
    auto trimmed = query->string().trimmed();
    auto url = QUrl::fromUserInput(trimmed);

    // Check syntax and TLD validity
    if (!url.isValid())
        return results;

    if ((url.scheme() == "http" || url.scheme() == "https")){

        auto tld = url.host().section('.', -1, -1);

        // Skip tld only queries
        if (trimmed.size() == tld.size())
            return results;

        // validate top level domain if scheme is not given (http assumed)
        if (tld.size() == 0 || (!trimmed.startsWith("http") && !binary_search(valid_tlds.begin(), valid_tlds.end(), tld)))
            return results;

        results.emplace_back(
            StandardItem::make(
                "url_hanlder",
                tr("Open URL in browser"),
                tr("Open %1").arg(url.authority()),
                {"xdg:www", "xdg:web-browser", "xdg:emblem-web", ":default"},
                {
                    {
                        "open_url", tr("Open URL"),
                        [url](){ QDesktopServices::openUrl(url); }
                    }
                }
            ),
            1.0f
        );
    }
    return results;
}
