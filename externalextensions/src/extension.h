// Copyright (C) 2014-2018 Manuel Schneider

#pragma once
#include <QObject>
#include <memory>
#include "albert/extension.h"
#include "externalextension.h"

namespace ExternalExtensions {

class Private;

class Extension final :
        public Core::Extension
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID ALBERT_EXTENSION_IID FILE "metadata.json")

public:

    Extension();
    ~Extension();

    QString name() const override { return "External extensions"; }
    QWidget *widget(QWidget *parent = nullptr) override;

    void reloadExtensions();

private:

    std::unique_ptr<Private> d;

signals:

    void extensionsUpdated();

};
}
