// Copyright (C) 2014-2018 Manuel Schneider

#pragma once
#include <QObject>
#include <QProcess>
#include <QMutex>
#include <map>
#include "albert/queryhandler.h"

namespace ExternalExtensions {

class ExternalExtension final : public Core::QueryHandler
{
public:

    enum class State {
        Initialized,
        Error
    };


    ExternalExtension(const QString &path, const QString &id);
    ~ExternalExtension();

    /*
     * Implementation of extension interface
     */

    QStringList triggers() const override { return {trigger_}; }
    void handleQuery(Core::Query *query) const override;

    /*
     * Extension specific members
     */


    const QString &path() const { return path_; }
    const QString &id() const { return id_; }
    const QString &name() const { return name_; }
    const QString &author() const { return author_; }
    const QString &version() const { return version_; }
    const QString &description() const { return description_; }
    const QString &usageExample() const { return usageExample_; }
    const QString &trigger() const { return trigger_; }
    const QStringList &dependencies() const { return dependencies_; }
    const State  &state() const { return state_; }
    const QString  &errorString() const { return errorString_; }

private:

    QString runOperation(const QString &);

    QString path_;
    QString id_;
    QString name_;
    QString author_;
    QString version_;
    QString trigger_;
    QString description_;
    QString usageExample_;
    QStringList dependencies_;
    State state_;
    QString errorString_;
    mutable std::map<QString, QString> variables_;
    mutable QMutex processMutex_;
};

}
