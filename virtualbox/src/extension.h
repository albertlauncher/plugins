// Copyright (C) 2014-2015 Manuel Schneider
// Contributed to by 2016-2017 Martin Buergmann

#pragma once
#include <QObject>
#include <memory>
#include "core/extension.h"
#include "core/queryhandler.h"

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
    ~Extension();

    QString name() const override { return tr("VirtualBox"); }
    QWidget *widget(QWidget *parent = nullptr) override;
    void setupSession() override;
    void handleQuery(Core::Query *) const override;

private:

    std::unique_ptr<Private> d;

};
}
