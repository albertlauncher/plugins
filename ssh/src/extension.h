// Copyright (C) 2014-2018 Manuel Schneider

#pragma once
#include <QObject>
#include <memory>
#include "core/extension.h"
#include "core/queryhandler.h"

namespace Ssh {

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

    QString name() const override { return "SecureShell"; }
    QWidget *widget(QWidget *parent = nullptr) override;
    QStringList triggers() const override { return {"ssh "}; }
    void handleQuery(Core::Query * query) const override;

    void rescan();

    bool useKnownHosts();
    void setUseKnownHosts(bool b = true);

private:

    std::unique_ptr<Private> d;

};
}
