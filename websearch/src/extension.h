// Copyright (C) 2014-2018 Manuel Schneider

#pragma once
#include <QLoggingCategory>
#include <memory>
#include "albert/extension.h"
#include "albert/queryhandler.h"
#include "albert/fallbackprovider.h"
#include "searchengine.h"
Q_DECLARE_LOGGING_CATEGORY(qlc)

namespace Websearch {

class Private;

class Extension final :
        public Core::Extension,
        public Core::QueryHandler,
        public Core::FallbackProvider
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID ALBERT_EXTENSION_IID FILE "metadata.json")

public:

    Extension();
    ~Extension();

    QString name() const override { return "Websearch"; }
    QWidget *widget(QWidget *parent = nullptr) override;
    QStringList triggers() const override;
    void handleQuery(Core::Query * query) const override;
    std::vector<std::shared_ptr<Core::Item>> fallbacks(const QString &) override;

    const std::vector<SearchEngine>& engines() const;
    void setEngines(const std::vector<SearchEngine> &engines);

    void restoreDefaultEngines();

private:

    std::unique_ptr<Private> d;

signals:

    void enginesChanged(const std::vector<SearchEngine> &engines);

};

}
