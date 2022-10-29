// Copyright (c) 2022 Manuel Schneider

#include <QRegularExpression>
#include <QDesktopServices>
#include <QUrl>
#include "plugin.h"
using namespace std;
using namespace albert;

void ::Plugin::handleQuery(Query &query) const
{
    QUrl url = QUrl::fromUserInput(query.string().trimmed());

    // Check syntax and TLD validity
    if ( url.isValid() && ( query.string().startsWith("http://") ||  // explict scheme
                            query.string().startsWith("https://") ||  // explict scheme
                            ( QRegularExpression(R"R(\S+\.\S+$)R").match(url.host()).hasMatch()))) {// &&
        //!url.topLevelDomain().isNull()) ) ) {  // valid TLD

        query.add(StandardItem::make(
                "url_hanlder",
                "Open URL in browser",
                QString("Visit %1").arg(url.authority()),
                {"xdg:www", "xdg:web-browser", "xdg:emblem-web", ":default"},
                {{"open_url","Open URL", [url](){ QDesktopServices::openUrl(url); }}}
        ));
    }
}
