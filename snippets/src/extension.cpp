// Copyright (C) 2014-2018 Manuel Schneider

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
#include "util/standardactions.h"
#include "util/standarditem.h"
#include "extension.h"
#include <mutex>
using namespace std;
using namespace Core;

namespace {
const QString trigger = "snip ";
mutex db_mutex;
}


class Snippets::Private
{
public:
    QPointer<ConfigWidget> widget;
    QSqlDatabase db;
};


/** ***************************************************************************/
Snippets::Extension::Extension()
    : Core::Extension("org.albert.extension.snippets"), // Must match the id in metadata
      Core::QueryHandler(Plugin::id()),
      d(new Private) {

    // Check if sqlite is available
    d->db = QSqlDatabase::addDatabase("QSQLITE", Plugin::id());
    if ( !d->db.isValid() )
        throw("No SQLite driver available");


    // Port config to new name
    QDir configDir(QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation));
    QString oldId = "org.albert.extension.kvstore";
    if (!configDir.exists(Plugin::id()) && configDir.exists(oldId)){

        // Rename database and folder
        configDir.cd(oldId);
        configDir.rename("kvstore.db", "snippets.db");

        // Rename config dir
        configDir.cdUp();
        configDir.rename(oldId, Plugin::id());

        d->db.setDatabaseName(configLocation().filePath("snippets.db"));
        if (!d->db.open())
            throw("Unable to establish a database connection.");

        // Create tables
        d->db.exec("CREATE TABLE snippets (title TEXT PRIMARY KEY, text NOT NULL);");
        d->db.exec("INSERT INTO snippets SELECT * FROM kv;");
        d->db.exec("DROP TABLE kv;");
    }

    d->db.setDatabaseName(configLocation().filePath("snippets.db"));
    if (!d->db.open())
        throw("Unable to establish a database connection.");

    // Create tables
    QSqlQuery q(d->db);
    if (!q.exec("CREATE TABLE IF NOT EXISTS snippets (title TEXT PRIMARY KEY, text NOT NULL);"))
        throw("Unable to create table.");

    registerQueryHandler(this);
}


/** ***************************************************************************/
Snippets::Extension::~Extension() {
    d->db.close();
}


/** ***************************************************************************/
QWidget *Snippets::Extension::widget(QWidget *parent) {
    if (d->widget.isNull()) {
        d->widget = new ConfigWidget(&d->db, parent);
    }
    return d->widget;
}

/** ***************************************************************************/
QStringList Snippets::Extension::triggers() const {
    return { trigger };
}


/** ***************************************************************************/
void Snippets::Extension::handleQuery(Core::Query * query) const {

//     if ( query->isTriggered() ) {

//        QString sec1 = query->string().section(' ', 0, 0, QString::SectionSkipEmpty);

//        // SETTING
//        if ( sec1 == "set" ){
//            QString key = query->string().section(' ', 1, 1, QString::SectionSkipEmpty);
//            QString value = query->string().section(' ', 2, -1, QString::SectionSkipEmpty);

//            if ( key.isEmpty() || value.isEmpty() )
//                return;

//            auto item = std::make_shared<StandardItem>();
//            item->setText(QString("Set '%1': '%2'").arg(key, value));
//            item->setSubtext(QString("Store this mapping in the database."));
//            item->setIconPath(":snippet");
//            item->setCompletion(query->string());
//            item->addAction(make_shared<FuncAction>("Add mapping to the database",
//                                                    [this, key, value](){
//                QSqlQuery q(d->db);
//                q.prepare(insertStmt);
//                q.bindValue(":key", key);
//                q.bindValue(":value", value);
//                q.exec();
//                if (this->d->widget)
//                    this->d->widget->updateTable();
//            }));

//            query->addMatch(move(item));
//        }

//        // UNSETTING
//        else if ( sec1 == "unset" ){
//            QString searchterm = query->string().section(' ', 1, -1, QString::SectionSkipEmpty);
//            QSqlQuery q(d->db);
//            q.exec(QString("SELECT key, value FROM kv WHERE key LIKE '%1%'").arg(searchterm));
//            while ( q.next() ){
//                QString key = q.value(0).toString();

//                auto item = std::make_shared<StandardItem>();
//                item->setText(QString("Unset '%1': '%2'").arg(key, q.value(1).toString()));
//                item->setSubtext(QString("Remove this mapping from the database."));
//                item->setIconPath(":snippet");
//                item->setCompletion(QString("%1 unset %2").arg(trigger, key));
//                item->addAction(make_shared<FuncAction>("Remove mapping from database",
//                                                        [this, key](){
//                    QSqlQuery q(d->db);
//                    q.prepare(removeStmt);
//                    q.bindValue(":key", key);
//                    q.exec();
//                    if (this->d->widget)
//                        this->d->widget->updateTable();
//                }));

//                query->addMatch(move(item), static_cast<uint>(1.0/key.length()*searchterm.length()));
//            }
//        }
//    }

    // LOOKUP

    // Allow empty lookup only for triggered queries
    if (query->string().trimmed().isEmpty() && !query->isTriggered())
        return;

    unique_lock<mutex> lock(db_mutex);

    QSqlQuery q(d->db);
    q.exec(QString("SELECT * FROM snippets WHERE title LIKE '%%%1%%'").arg(query->string()));
    QRegularExpression re(QString("(%1)").arg(query->string()), QRegularExpression::CaseInsensitiveOption);
    while ( q.next() ){
        QString key = q.value(0).toString();
        QString value = q.value(1).toString();

        auto item = std::make_shared<StandardItem>(QString("%1_%2").arg(Plugin::id(), key));
        item->setText(QString("Text snippet '%1'").arg(QString(key).replace(re, "<u>\\1</u>")));
        item->setSubtext("Copy the snippet to clipboard");
        item->setIconPath(":snippet");
        item->setCompletion(QString("%1%2").arg(trigger, key));
        item->addAction(make_shared<ClipAction>("Copy value to clipboard", value));

        query->addMatch(move(item), static_cast<uint>(1.0/key.length()*query->string().length()));
    }
}

