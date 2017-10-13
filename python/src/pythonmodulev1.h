// Copyright (c) 2017 Manuel Schneider

#pragma once
#include <QObject>
#include <QStringList>
#include <memory>

namespace Core {
class Query;
}

namespace Python {

class PythonModuleV1Private;

class PythonModuleV1 final : public QObject
{
    Q_OBJECT

public:

    enum class State { Unloaded, Loaded, Error };

    PythonModuleV1(const QString &path);
    ~PythonModuleV1();

    void load();
    void unload();

    void handleQuery(Core::Query * query) const;

    const QString &path() const;
    const QString &id() const;
    const QString &name() const;
    const QString &author() const;
    const QString &version() const;
    const QString &description() const;
    const QString &trigger() const;
    const QStringList &dependencies() const;
    const QString &errorString() const;
    State state() const;

private:

    std::unique_ptr<PythonModuleV1Private> d;

signals:

    void moduleChanged();

};

}
