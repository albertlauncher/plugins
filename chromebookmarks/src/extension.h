// Copyright (C) 2014-2018 Manuel Schneider

#pragma once
#include <QObject>
#include <memory>
#include "albert/extension.h"
#include "albert/queryhandler.h"

namespace ChromeBookmarks {

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

    QString name() const override { return "Chrome bookmarks"; }
    QWidget *widget(QWidget *parent = nullptr) override;
    void handleQuery(Core::Query * query) const override;

    const QString &path();
    void setPath(const QString &path);
    void restorePath();

    bool fuzzy();
    void setFuzzy(bool b = true);

    void updateIndex();

private:

    std::unique_ptr<Private> d;

signals:

    void pathChanged(const QString&);
    void statusInfo(const QString&);

};
}
