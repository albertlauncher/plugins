// Copyright (c) 2022-2024 Manuel Schneider

#include "themesqueryhandler.h"
#include "window.h"
#include <albert/standarditem.h>
#include <albert/util.h>
using namespace albert;
using namespace std;


ThemesQueryHandler::ThemesQueryHandler(Window *w) : window(w) {}

QString ThemesQueryHandler::id() const
{ return QStringLiteral("widgetsboxmodel.themes"); }

QString ThemesQueryHandler::name() const
{ return QStringLiteral("Themes"); }

QString ThemesQueryHandler::description() const
{ return Window::tr("Switch themes"); }

QString ThemesQueryHandler::defaultTrigger() const { return "theme "; }

void ThemesQueryHandler::handleTriggerQuery(Query *query)
{
    auto trimmed = query->string().trimmed();
    for (const auto &fi : window->findStyles())
    {
        auto name = fi.baseName();
        auto path = fi.canonicalFilePath();
        if (name.contains(trimmed, Qt::CaseInsensitive))
        {
            query->add(
                StandardItem::make(
                    QString("theme_%1").arg(name),
                    name,
                    path,
                    {"gen:?text=🎨"},
                    {
                        {
                            "apply", Window::tr("Apply theme"),
                            [=, this]{ window->setStyle(Style::read(path)); }
                        },
                        {
                            "setlight", Window::tr("Use in light mode"),
                            [=, this]{ window->set_light_style_file(path); }
                        },
                        {
                            "setdark", Window::tr("Use in dark mode"),
                            [=, this]{ window->set_dark_style_file(path); }
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

