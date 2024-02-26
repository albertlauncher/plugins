// Copyright (c) 2023-2024 Manuel Schneider

#include "albert/albert.h"
#include "albert/extension/frontend/query.h"
#include "albert/logging.h"
#include "plugin.h"
#include "qmlinterface.h"

#include <QStringListModel>
using namespace albert;

QmlInterface::QmlInterface(Plugin *plugin) : plugin_(plugin) { }

void QmlInterface::showSettings()
{
    plugin_->setVisible(false);
    albert::showSettings();
}

QObject *QmlInterface::currentQuery() { return currentQuery_; }

QString QmlInterface::kcString(int kc) { return QKeySequence(kc).toString(); }

void QmlInterface::debug(QString m) { DEBG << m; }
void QmlInterface::info(QString m) { INFO << m; }
void QmlInterface::warning(QString m) { WARN << m; }
void QmlInterface::critical(QString m) { CRIT << m; }

QAbstractListModel *QmlInterface::createStringListModel(const QStringList &action_names) const
{ return new QStringListModel(action_names); }

void QmlInterface::setQuery(Query *q)
{
    CRIT << "QmlInterface::setQuery";

    if(currentQuery_)
        disconnect(currentQuery_, &Query::finished,
                   this, &QmlInterface::currentQueryFinished);

    CRIT << "das";

    // important for qml ownership determination
    if (q)
        q->setParent(this);
    CRIT << "noch";

    currentQuery_ = q;
    emit currentQueryChanged();
    CRIT << "geschafft";


    if (q)
    {
        if (q->isFinished())
            emit currentQueryFinished();
        else
            connect(q, &Query::finished, this, &QmlInterface::currentQueryFinished);
    }

    CRIT << "!";
}
