// Copyright (C) 2014-2019 Manuel Schneider

#pragma once
#include <QObject>
#include <memory>
#include "albert/extension.h"
#include "albert/queryhandler.h"

namespace Applications {

class Private;

class Extension final : public Core::Extension, public Core::QueryHandler
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID ALBERT_EXTENSION_IID FILE "metadata.json")

public:

    Extension();
    ~Extension() override;

    QString name() const override { return "Applications"; }
    QWidget *widget(QWidget *parent = nullptr) override;
    void handleQuery(Core::Query * query) const override;

    bool fuzzy();
    void setFuzzy(bool b = true);

    void updateIndex();

private:

    std::unique_ptr<Private> d;

signals:

    void statusInfo(const QString&);

};
}
