// Copyright (c) 2022-2024 Manuel Schneider

#include "themesqueryhandler.h"
#include "window.h"
#include <albert/standarditem.h>
#include <albert/util.h>
using namespace albert;
using namespace std;


ThemesQueryHandler::ThemesQueryHandler(Window *w) : window(w) {}

QString ThemesQueryHandler::id() const
{ return QStringLiteral("widgetboxmodel_themes"); }

QString ThemesQueryHandler::name() const
{ return QStringLiteral("Themes"); }

QString ThemesQueryHandler::description() const
{ return Window::tr("Switch themes"); }

QString ThemesQueryHandler::defaultTrigger() const { return "theme "; }

void ThemesQueryHandler::handleTriggerQuery(Query *query)
{
    auto trimmed = query->string().trimmed();
    for (const auto &[name, path] : window->themes)
    {
        if (name.contains(trimmed, Qt::CaseInsensitive))
        {
            query->add(
                StandardItem::make(
                    QString("theme_%1").arg(name),
                    name,
                    path,
                    {":app_icon"},
                    {
                        {
                            "apply", Window::tr("Apply theme"),
                            [w=window, n=name](){ w->applyThemeFile(w->themes.at(n)); }
                        },
                        {
                            "setlight", Window::tr("Use in light mode"),
                            [w=window, n=name](){ w->setLightTheme(n); }
                        },
                        {
                            "setdark", Window::tr("Use in dark mode"),
                            [w=window, n=name](){ w->setDarkTheme(n); }
                        },
                        {
                            "open", Window::tr("Open theme file"),
                            [p=path](){ openUrl("file://" + p); }
                        }
                    }
                )
            );
        }
    }
}

