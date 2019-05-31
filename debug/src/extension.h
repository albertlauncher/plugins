// Copyright (C) 2014-2018 Manuel Schneider

#pragma once
#include <QObject>
#include <memory>
#include "albert/extension.h"
#include "albert/queryhandler.h"

namespace Debug {

class Private;

class Extension final :
        public Core::Extension,
        public Core::QueryHandler
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID ALBERT_EXTENSION_IID FILE "metadata.json")

public:

    Extension();
    ~Extension();

    QString name() const override { return "Debug"; }
    QWidget *widget(QWidget *parent = nullptr) override;
    QStringList triggers() const override;
    void handleQuery(Core::Query * query) const override;
    ExecutionType executionType() const override;

    int count() const;
    void setCount(const int &count);

    bool async() const;
    void setAsync(bool async);

    int delay() const;
    void setDelay(const int &delay);

    const QString& trigger() const;
    void setTrigger(const QString &trigger);

private:

    std::unique_ptr<Private> d;

};
}
