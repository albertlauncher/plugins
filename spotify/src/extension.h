// Copyright (C) 2014-2021 Manuel Schneider, Ivo Šmerek

#pragma once
#include <QLoggingCategory>
#include <QObject>
#include <memory>
#include "albert/extension.h"
#include "albert/queryhandler.h"
Q_DECLARE_LOGGING_CATEGORY(qlc)

namespace Spotify {

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

    QString name() const override { return "Spotify"; }
    QWidget *widget(QWidget *parent = nullptr) override;
//    QStringList triggers() const override { return {"spotify "}; }
    void setupSession() override;
    void teardownSession() override;
    void handleQuery(Core::Query * query) const override;

private:
    std::unique_ptr<Private> d;
    QString COVERS_DIR_PATH = "/tmp/albert-spotify-covers";
};
}