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

    enum class State { InvalidMetadata, MissingDeps, Unloaded, Loaded, Error };

    PythonModuleV1(const QString &path);
    ~PythonModuleV1();

    void load();
    void unload();

    void handleQuery(Core::Query * query) const;

    QString path() const;
    QString sourcePath() const;

    QString id() const;
    QString name() const;
    QString description() const;
    QString version() const;
    QStringList authors() const;
    QStringList executableDependecies() const;
    QStringList pythonDependecies() const;
    QStringList triggers() const;

    State state() const;
    QString errorString() const;

private:

    void readMetadata();

    std::unique_ptr<PythonModuleV1Private> d;

};

}
