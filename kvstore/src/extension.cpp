// albert - a simple application launcher for linux
// Copyright (C) 2014-2015 Manuel Schneider
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

#include <QApplication>
#include <QClipboard>
#include <QPointer>
#include <QSqlDatabase>
#include <QSqlDriver>
#include <QSqlError>
#include <QSqlQuery>
#include <QCryptographicHash>
#include <stdexcept>
#include "configwidget.h"
#include "core/item.h"
#include "core/query.h"
#include "util/standarditem.h"
#include "util/standardaction.h"
#include "extension.h"
using namespace std;
using namespace Core;

namespace {
const QString insertStmt = "INSERT OR REPLACE INTO kv (key, value) VALUES (:key, :value);";
const QString removeStmt = "DELETE FROM kv WHERE key=:key;";
}


class KeyValueStore::Private
{
public:
    QPointer<ConfigWidget> widget;
    QSqlDatabase db;
};


/** ***************************************************************************/
KeyValueStore::Extension::Extension()
    : Core::Extension("org.albert.extension.kvstore"), // Must match the id in metadata
      Core::QueryHandler(Plugin::id()),
      d(new Private) {

    d->db = QSqlDatabase::addDatabase("QSQLITE", Plugin::id());
    if ( !d->db.isValid() )
        throw("No SQLite driver available");

    d->db.setDatabaseName(configLocation().filePath("kvstore.db"));
    if (!d->db.open())
        throw("Unable to establish a database connection.");

    d->db.transaction();

    // Create tables
    QSqlQuery q(d->db);
    if (!q.exec("CREATE TABLE IF NOT EXISTS kv (key TEXT PRIMARY KEY, value NOT NULL);"))
        throw("Unable to create table.");

    d->db.commit();


    registerQueryHandler(this);
}


/** ***************************************************************************/
KeyValueStore::Extension::~Extension() {

}


/** ***************************************************************************/
QWidget *KeyValueStore::Extension::widget(QWidget *parent) {
    if (d->widget.isNull()) {
        d->widget = new ConfigWidget(parent);
    }
    return d->widget;
}


/** ***************************************************************************/
void KeyValueStore::Extension::handleQuery(Core::Query * query) const {

     if ( query->isTriggered() ) {

        QString sec1 = query->string().section(' ', 0, 0, QString::SectionSkipEmpty);

        // SETTING
        if ( sec1 == "set" ){
            QString key = query->string().section(' ', 1, 1, QString::SectionSkipEmpty);
            QString value = query->string().section(' ', 2, -1, QString::SectionSkipEmpty);

            if ( key.isEmpty() || value.isEmpty() )
                return;

            shared_ptr<StandardItem> item = std::make_shared<StandardItem>();
            item->setText(QString("Set '%1': '%2'").arg(key, value));
            item->setSubtext(QString("Store this mapping in the database."));
            item->setIconPath(":kv");
            item->setCompletionString(query->string());
            item->setActions({std::make_shared<StandardAction>(
                              QString("Add mapping to the database"),
                              [=](){
                                  QSqlQuery q(d->db);
                                  q.prepare(insertStmt);
                                  q.bindValue(":key", key);
                                  q.bindValue(":value", value);
                                  q.exec();
                              })
                             });
            query->addMatch(move(item));
        }

        // UNSETTING
        else if ( sec1 == "unset" ){
            QString searchterm = query->string().section(' ', 1, -1, QString::SectionSkipEmpty);
            QSqlQuery q(d->db);
            q.exec(QString("SELECT key, value FROM kv WHERE key LIKE '%1%'").arg(searchterm));
            while ( q.next() ){
                QString key = q.value(0).toString();

                shared_ptr<StandardAction> action = std::make_shared<StandardAction>();
                action->setText("Remove mapping from database");
                action->setAction([=](){
                    QSqlQuery q(d->db);
                    q.prepare(removeStmt);
                    q.bindValue(":key", key);
                    q.exec();
                });

                shared_ptr<StandardItem> item = std::make_shared<StandardItem>();
                item->setText(QString("Unset '%1': '%2'").arg(key, q.value(1).toString()));
                item->setSubtext(QString("Remove this mapping from the database."));
                item->setIconPath(":kv");
                item->setCompletionString(QString("kv unset %1").arg(key));
                item->setActions({action});
                query->addMatch(move(item), static_cast<uint>(1.0/key.length()*searchterm.length()));
            }
        }
    }

    // LOOKUP

    // Allow empty lookup (getting everything) only for triggered queries
    if (query->string().isEmpty() && query->trigger().isEmpty())
        return;

    QSqlQuery q(d->db);
    q.exec(QString("SELECT key, value FROM kv WHERE key LIKE '%1%'").arg(query->string()));
    while ( q.next() ){
        QString key = q.value(0).toString();
        QString value = q.value(1).toString();

        shared_ptr<StandardAction> action = std::make_shared<StandardAction>();
        action->setText("Copy value to clipboard");
        action->setAction([=](){ QApplication::clipboard()->setText(value); });

        shared_ptr<StandardItem> item = std::make_shared<StandardItem>(QString("kv_%1").arg(key));
        item->setText(value);
        item->setSubtext(QString("Value of '%1'").arg(key));
        item->setIconPath(":kv");
        item->setCompletionString(QString("kv %1").arg(key));
        item->setActions({action});
        query->addMatch(move(item), static_cast<uint>(1.0/key.length()*query->string().length()));
    }
}

