// Copyright (c) 2017 Manuel Schneider

#pragma once
#include <QObject>
#include <memory>
#include "core/extension.h"
#include "core/queryhandler.h"

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
    ~Extension();

    QString name() const override { return "Python extensions"; }
    QWidget *widget(QWidget *parent = nullptr) override;
    void handleQuery(Core::Query * query) const override;
    QStringList triggers() const override;

    const std::vector<std::unique_ptr<PythonModuleV1>> &modules();

    bool isEnabled(PythonModuleV1 &module);
    void setEnabled(PythonModuleV1 &module, bool enabled);

private:

    std::unique_ptr<Private> d;

    void updateDirectory(const QString &path);

signals:

    void modulesChanged();
};
}
