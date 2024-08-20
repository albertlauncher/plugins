// Copyright (c) 2022-2024 Manuel Schneider

#include "docitem.h"
#include "docset.h"
#include "plugin.h"
#include <QRegularExpression>
#include <QSqlDatabase>
#include <QSqlDriver>
#include <QSqlError>
#include <QSqlQuery>
#include <QXmlStreamReader>
#include <albert/logging.h>
#include <set>
using namespace albert;
using namespace std;


Docset::Docset(QString n, QString t, QString sid, QString ip)
    : name(n), title(t), source_id(sid), icon_path(ip) {}

bool Docset::isInstalled() const { return !path.isNull(); }

void Docset::createIndexItems(vector<IndexItem> &results) const
{
    // Fixes strings and makes sure to use implicitly shared strings
    struct StringProcessor
    {
        StringProcessor(const Docset &ds, vector<IndexItem> &ii):
            docset(ds), index_items(ii) {}

        void add(const QString &t, const QString &n, QString p, const QString &a)
        {
            auto item = make_shared<DocItem>(docset,
                                             shared(t),
                                             shared(n),
                                             shared(QString(p).remove(dashEntryRegExp)),
                                             shared(a.section("/", -1)));
            index_items.emplace_back(item, item->text());
        }

    private:

        const QString &shared(const QString &string)
        { return *shared_strings.emplace(string).first; }

        const Docset &docset;
        vector<IndexItem> &index_items;
        QRegularExpression dashEntryRegExp{"<dash_entry_.*>"};
        set<QString> shared_strings;

    } sp(*this, results);


    if (auto file_path = QString("%1/Contents/Resources/Tokens.xml").arg(path);
        QFile::exists(file_path))
    {
        INFO << "Indexing docset" << file_path;

        QFile f(file_path);
        if (!f.open(QIODevice::ReadOnly|QIODevice::Text))
        {
            WARN << f.errorString();
            return;
        }

        QXmlStreamReader xml(&f);
        xml.readNext();

        while (!xml.atEnd() && !xml.hasError())
        {
            auto tokenType = xml.readNext();
            if (tokenType == QXmlStreamReader::StartElement && xml.name() == QLatin1String("Token"))
            {
                QString t, n, p, a;

                for (;!(xml.tokenType() == QXmlStreamReader::EndElement && xml.name() == QLatin1String("Token")); xml.readNext())
                {
                    if (xml.tokenType() == QXmlStreamReader::StartElement)
                    {
                        if (xml.name() == QLatin1String("TokenIdentifier"))
                        {
                            for (;!(xml.tokenType() == QXmlStreamReader::EndElement && xml.name() == QLatin1String("TokenIdentifier")); xml.readNext())
                            {
                                if (xml.name() == QLatin1String("Name"))
                                    n = xml.readElementText();
                                else if (xml.name() == QLatin1String("Type"))
                                    t = xml.readElementText();
                            }
                        }
                        else if (xml.name() == QLatin1String("Path"))
                            p = xml.readElementText();
                        else if (xml.name() == QLatin1String("Anchor"))
                            a = xml.readElementText();
                    }
                }

                sp.add(t, n, p, a);
            }
        }
        f.close();
    }
    else if (file_path = QString("%1/Contents/Resources/docSet.dsidx").arg(path); QFile::exists(file_path))
    {
        INFO << "Indexing docset" << file_path;

        {
            QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", Plugin::instance()->id());
            db.setDatabaseName(file_path);
            if (!db.open())
            {
                WARN << "Unable to open database connection" << db.databaseName();
                return;
            }
            else if(QSqlQuery sql(db);
                    sql.exec("SELECT name FROM sqlite_master WHERE type='table' AND name='searchIndex'"))
            {
                if (sql.next()) // returns true if searchIndex exists
                {
                    if (sql.exec("SELECT name, type, path FROM searchIndex ORDER BY name;"))
                        while (sql.next())
                        {
                            auto n = sql.value(0).toString();
                            auto t = sql.value(1).toString();
                            auto pa = sql.value(2).toString().split("#");

                            if (pa.size() == 2)
                                sp.add(t, n, pa[0], pa[1]);
                            else if (pa.size() == 1)
                                sp.add(t, n, pa[0], {});
                            else {
                                WARN << "Invalid path" << pa;
                                continue;
                            }
                        }
                    else
                        WARN << sql.lastQuery() << sql.lastError().text();
                }
                else
                {
                    if(sql.exec(R"R(
                        SELECT
                            ztypename, ztokenname, zpath, zanchor
                        FROM ztoken
                            INNER JOIN ztokenmetainformation ON ztoken.zmetainformation = ztokenmetainformation.z_pk
                            INNER JOIN zfilepath ON ztokenmetainformation.zfile = zfilepath.z_pk
                            INNER JOIN ztokentype ON ztoken.ztokentype = ztokentype.z_pk
                        ORDER BY ztokenname;
                    )R"))
                        while (sql.next())
                            sp.add(sql.value(0).toString(),
                                   sql.value(1).toString(),
                                   sql.value(2).toString(),
                                   sql.value(3).toString());
                    else
                        WARN << sql.lastQuery() << sql.lastError().text();
                }
            }
            else
                WARN << sql.lastQuery() << sql.lastError().text();

            db.close();
        }
        QSqlDatabase::removeDatabase(Plugin::instance()->id());
    }
    else
        WARN << "No index found in" << file_path;
}
