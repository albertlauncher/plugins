// Copyright (C) 2014-2018 Manuel Schneider

#pragma once
#include <QObject>
#include "core/extension.h"
#include "core/queryhandler.h"
#include <memory>

namespace FirefoxBookmarks {

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

    QString name() const override { return "Firefox bookmarks"; }
    QWidget *widget(QWidget *parent = nullptr) override;
    void handleQuery(Core::Query * query) const override;

    void setProfile(const QString &profile);
    void changeFuzzyness(bool fuzzy);
    void changeOpenPolicy(bool withFirefox);

private:

    std::unique_ptr<Private> d;

signals:

    void statusInfo(const QString&);
};
}
