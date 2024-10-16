// Copyright (c) 2023-2024 Manuel Schneider

#pragma once
#include <QAbstractListModel>
#include <QObject>
#include <QString>
class Plugin;
namespace albert { class Query; }


class QmlInterface : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QObject* current_query_ READ currentQuery  NOTIFY queryChanged)

public:
    QmlInterface(Plugin *plugin);

    Q_INVOKABLE void showSettings();
    Q_INVOKABLE QObject *currentQuery();
    Q_INVOKABLE QString kcString(int kc);

    Q_INVOKABLE void debug(QString m);
    Q_INVOKABLE void info(QString m);
    Q_INVOKABLE void warning(QString m);
    Q_INVOKABLE void critical(QString m);

    Q_INVOKABLE QAbstractListModel *createStringListModel(const QStringList &action_names) const;

    void setQuery(albert::Query *q);

private:
    Plugin *plugin_;
    albert::Query *current_query_;

signals:
    void queryChanged();
    // convenience signals to avoid the boilerplate in qml
    void queryMatchesAdded();
    void queryFinished();

};

