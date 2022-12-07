// Copyright (c) 2022 Manuel Schneider

#pragma once
#include <QLoggingCategory>
#include <QObject>
#include "albert/extension.h"
#include "albert/queryhandler.h"
#include <memory>
Q_DECLARE_LOGGING_CATEGORY(qlc)

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

    void setProfile(int);
    void setProfile(const QString&);
    void changeFuzzyness(bool fuzzy);
    void changeOpenPolicy(bool withFirefox);

private:

    std::unique_ptr<Private> d;

signals:

    void statusInfo(const QString&);
};
}
