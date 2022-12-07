// Copyright (c) 2022 Manuel Schneider

#pragma once
#include <QObject>
#include <QLoggingCategory>
#include <memory>
#include "albert/extension.h"
#include "albert/queryhandler.h"
Q_DECLARE_LOGGING_CATEGORY(qlc)

namespace VirtualBox {

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

    QString name() const override { return tr("VirtualBox"); }
    QWidget *widget(QWidget *parent = nullptr) override;
    void setupSession() override;
    void handleQuery(Core::Query *) const override;

private:

    std::unique_ptr<Private> d;

};
}
