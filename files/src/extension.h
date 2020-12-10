// Copyright (C) 2014-2018 Manuel Schneider

#pragma once
#include <QLoggingCategory>
#include <QObject>
#include <memory>
#include "albert/extension.h"
#include "albert/queryhandler.h"
Q_DECLARE_LOGGING_CATEGORY(qlc)

namespace Files {

class Private;

class Extension final :
        public Core::Extension,
        public Core::QueryHandler
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID ALBERT_EXTENSION_IID FILE "metadata.json")
    Q_PROPERTY(QStringList paths READ paths WRITE setPaths NOTIFY pathsChanged)

public:

    Extension();
    ~Extension();

    QString name() const override { return "Files"; }
    QStringList triggers() const override { return {"/", "~"}; }
    QWidget *widget(QWidget *parent = nullptr) override;
    void handleQuery(Core::Query * query) const override;

    const QStringList &paths() const;
    void setPaths(const QStringList &);
    void restorePaths();

    bool indexHidden() const;
    void setIndexHidden(bool b = true);

    bool followSymlinks() const;
    void setFollowSymlinks(bool b = true);

    uint scanInterval() const;
    void setScanInterval(uint minutes);

    bool fuzzy() const;
    void setFuzzy(bool b = true);

    QStringList filters() const;
    void setFilters(const QStringList &);

    void updateIndex();

private:

    std::unique_ptr<Private> d;

signals:

    void pathsChanged(const QStringList&);
    void statusInfo(const QString&);

};
}
