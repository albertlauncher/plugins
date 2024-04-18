// Copyright (c) 2023-2024 Manuel Schneider

#include "plugin.h"
#include "qmlinterface.h"
#include <QStringListModel>
#include <albert/logging.h>
#include <albert/query.h>
#include <albert/util.h>
using namespace albert;

QmlInterface::QmlInterface(Plugin *plugin):
    plugin_{plugin},
    current_query_{nullptr}

{ }

void QmlInterface::showSettings()
{
    plugin_->setVisible(false);
    albert::showSettings();
}

QObject *QmlInterface::currentQuery() { return current_query_; }

QString QmlInterface::kcString(int kc) { return QKeySequence(kc).toString(); }

void QmlInterface::debug(QString m) { DEBG << m; }
void QmlInterface::info(QString m) { INFO << m; }
void QmlInterface::warning(QString m) { WARN << m; }
void QmlInterface::critical(QString m) { CRIT << m; }

QAbstractListModel *QmlInterface::createStringListModel(const QStringList &action_names) const
{ return new QStringListModel(action_names); }

void QmlInterface::setQuery(Query *q)
{
    if(current_query_)
        disconnect(current_query_, nullptr, this, nullptr);

    current_query_ = q;
    emit queryChanged();

    if (q)
    {
        connect(q->matches(), &QAbstractItemModel::rowsInserted,
                this, &QmlInterface::queryMatchesAdded);
        connect(q, &Query::finished,
                this, &QmlInterface::queryFinished);
    }
}
