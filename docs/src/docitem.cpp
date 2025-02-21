// Copyright (c) 2022-2024 Manuel Schneider

#include "docitem.h"
#include "docset.h"
#include "plugin.h"
#include <QDir>
#include <QFile>
#include <QRegularExpression>
#include <QTextStream>
#include <albert/albert.h>
#include <albert/logging.h>
using namespace albert;
using namespace std;


DocItem::DocItem(const Docset &ds, const QString &t, const QString &n, const QString &p, const QString &a)
    : docset(ds), type(t), name(n), path(p), anchor(a) {}

QString DocItem::id() const
{ return docset.name + name; }

QString DocItem::text() const
{ return name; }

QString DocItem::subtext() const
{ return QString("%1 %2").arg(docset.title, type); }

QStringList DocItem::iconUrls() const
{ return { "file:" + docset.icon_path }; }

QString DocItem::inputActionText() const
{ return name; }

vector<Action> DocItem::actions() const
{ return {{ id(), Plugin::tr("Open documentation"), [this] { open(); } }}; }

// Workaround for some browsers not opening "file:" urls having an anchor
void DocItem::open() const
{
    // QTemporaryFile will not work here because its deletion introduces race condition

    if (QFile file(QDir(Plugin::instance()->cacheLocation()).filePath("trampoline.html"));
            file.open(QIODevice::WriteOnly))
    {
        auto url = QString("file:%1/Contents/Resources/Documents/%2").arg(docset.path, path);
        if (!anchor.isEmpty())
            url += "#" + anchor;

        QTextStream stream(&file);
        stream << QString(R"(<html><head><meta http-equiv="refresh" content="0;%1"></head></html>)")
                  .arg(url);
        file.close();

        ::open(file.fileName());
    }
    else
        WARN << "Failed to open file for writing" << file.fileName() << file.errorString();
}
