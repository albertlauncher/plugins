// Copyright (c) 2022-2024 Manuel Schneider

#pragma once
#include "application.h"
#include <QCoreApplication>
#include <QStringList>

class Terminal : public Application
{
    Q_DECLARE_TR_FUNCTIONS(Terminal)

public:

    Terminal(const ::Application &app, const QStringList &exec_arg);

    using ::Application::launch;

    void launch(const QString &script) const;
    void launch(QStringList commandline, const QString &working_dir = {}) const;

private:

    QStringList exec_arg_;

};
