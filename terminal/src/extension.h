// Copyright (C) 2014-2018 Manuel Schneider

#pragma once
#include <QObject>
#include <memory>
#include "albert/extension.h"
#include "albert/queryhandler.h"

namespace Terminal {

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

    QString name() const override { return "Terminal"; }
    QWidget *widget(QWidget *parent = nullptr) override;
    QStringList triggers() const override { return {">"}; }
    void handleQuery(Core::Query * query) const override;

private:

    void rebuildIndex();

    std::unique_ptr<Private> d;

};
}
