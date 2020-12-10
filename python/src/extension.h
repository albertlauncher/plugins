// Copyright (c) 2017-2018 Manuel Schneider

#pragma once
#include <QLoggingCategory>
#include <QObject>
#include <memory>
#include "albert/extension.h"
#include "albert/queryhandler.h"
Q_DECLARE_LOGGING_CATEGORY(qlc)

namespace Python {

class Private;
class PythonModuleV1;

class Extension final :
        public Core::Extension,
        public Core::QueryHandler
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID ALBERT_EXTENSION_IID FILE "metadata.json")

public:

    Extension();
    ~Extension() override;

    QString name() const override { return "Python extensions"; }
    QWidget *widget(QWidget *parent = nullptr) override;
    void handleQuery(Core::Query * query) const override;
    QStringList triggers() const override;

    const std::vector<std::unique_ptr<PythonModuleV1>> &modules();

    bool isEnabled(PythonModuleV1 &module);
    void setEnabled(PythonModuleV1 &module, bool enabled);

private:

    std::unique_ptr<Private> d;

    void reloadModules();

signals:

    void modulesChanged();
};
}
