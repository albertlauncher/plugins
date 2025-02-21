// Copyright (c) 2022-2024 Manuel Schneider

#include "terminal.h"
#include <QMessageBox>
#include <albert/albert.h>
#include <albert/logging.h>
#include <pwd.h>
#include <unistd.h>

Terminal::Terminal(const ::Application &app, const QStringList &exec_arg):
    ::Application(app), exec_arg_(exec_arg) {}

void Terminal::launch(const QString &script) const
{
    if (passwd *pwd = getpwuid(geteuid()); pwd == nullptr)
    {
        static const char* msg =
            QT_TR_NOOP("Failed to run terminal with script: getpwuid(…) failed.");
        WARN << msg;
        QMessageBox::warning(nullptr, {}, tr(msg));
    }

    else if (auto s = script.simplified(); s.isEmpty())
    {
        static const char* msg =
            QT_TR_NOOP("Failed to run terminal with script: Script is empty.");
        WARN << msg;
        QMessageBox::warning(nullptr, {}, tr(msg));
    }

    else
        launch(QStringList() << pwd->pw_shell << "-i" << "-c" << script);
}

void Terminal::launch(QStringList commandline, const QString &working_dir) const
{
    launchExec(QStringList() << exec() << exec_arg_ << commandline, {}, working_dir);
}
