// Copyright (c) 2017-2018 Manuel Schneider

#pragma once
#include <QLoggingCategory>
#include <QStringList>
#include <memory>

Q_DECLARE_LOGGING_CATEGORY(qlc_python_modulev1)

namespace Core {
class Query;
}

namespace Python {

class PythonModuleV1Private;

class PythonModuleV1 final
{

public:

    enum class State { InvalidMetadata, Unloaded, Loaded, Error };

    PythonModuleV1(const QString &path);
    ~PythonModuleV1();

    void load();
    void unload();

    void handleQuery(Core::Query * query) const;

    const QString &path() const;
    const QString &sourcePath() const;
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

    void readMetadata();

    std::unique_ptr<PythonModuleV1Private> d;

};

}
