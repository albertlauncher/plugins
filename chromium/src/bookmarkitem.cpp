// Copyright (c) 2022-2024 Manuel Schneider

#include "bookmarkitem.h"
#include <QCoreApplication>
#include <albert/albert.h>
using namespace albert;
using namespace std;

BookmarkItem::BookmarkItem(const QString &i, const QString &n, const QString &f, const QString &u):
    id_(i), name_(n), folder_(f), url_(u) {}

QString BookmarkItem::id() const
{ return id_; }

QString BookmarkItem::text() const
{ return name_; }

QString BookmarkItem::subtext() const
{ return QStringLiteral("[%1] %2").arg(folder_, url_); }

QString BookmarkItem::inputActionText() const
{ return name_; }

QStringList BookmarkItem::iconUrls() const
{
    static const QStringList icon_urls = {
#if defined Q_OS_UNIX and not defined Q_OS_MAC
        "xdg:www",
        "xdg:web-browser",
        "xdg:emblem-web",
#endif
        "qrc:star"
    };
    return icon_urls;
}

vector<Action> BookmarkItem::actions() const
{
    static const auto tr_open = QCoreApplication::translate("BookmarkItem", "Open URL");
    static const auto tr_copy = QCoreApplication::translate("BookmarkItem", "Copy URL to clipboard");
    return {
        {"open-url", tr_open, [this]() { openUrl(url_); }},
        {"copy-url", tr_copy, [this]() { setClipboardText(url_); }}
    };
}


