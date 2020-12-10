// Copyright (C) 2014-2018 Manuel Schneider

#pragma once
#include <QLoggingCategory>
#include <QObject>
#include <memory>
#include "albert/extension.h"
#include "albert/queryhandler.h"
Q_DECLARE_LOGGING_CATEGORY(qlc)

namespace ProjectNamespace {

class Private;

class Extension final :
        public Core::Extension,
        public Core::QueryHandler
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID ALBERT_EXTENSION_IID FILE "metadata.json")

public:

    Extension();
    ~Extension() override;

    QString name() const override { return "Template"; }
    QWidget *widget(QWidget *parent = nullptr) override;
    void setupSession() override;
    void teardownSession() override;
    void handleQuery(Core::Query * query) const override;

private:

    std::unique_ptr<Private> d;

};
}
