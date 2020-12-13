// Copyright (C) 2014-2018 Manuel Schneider

#pragma once
#include <QLoggingCategory>
#include <QObject>
#include <memory>
#include "albert/extension.h"
#include "albert/queryhandler.h"
Q_DECLARE_LOGGING_CATEGORY(qlc)

namespace Chromium {

class Private;

class Extension final : public Core::Extension, public Core::QueryHandler
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID ALBERT_EXTENSION_IID FILE "metadata.json")

public:

    Extension();
    ~Extension();

    QString name() const override { return "Chromium"; }
    QWidget *widget(QWidget *parent) override;
    void handleQuery(Core::Query *query) const override;

    void updatePaths();

    bool fuzzy();
    void setFuzzy(bool b = true);

private:

    std::unique_ptr<Private> d;

signals:

    void statusInfo(const QString&);

};
}
