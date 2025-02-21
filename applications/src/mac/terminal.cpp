// Copyright (c) 2022-2024 Manuel Schneider

#include "terminal.h"
#include <QDir>
#include <QFile>
#include <QMessageBox>
#include <albert/albert.h>
#include <albert/logging.h>
#include <pwd.h>
#include <unistd.h>
using namespace albert;

Terminal::Terminal(const ::Application &app, const char *apple_script):
    Application(app),
    apple_script_(apple_script)
{}

void Terminal::launch(QString script) const
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

    else if (QFile file(QDir(cacheLocation()).filePath("terminal_command"));
             !file.open(QIODevice::WriteOnly))
    {
        static const char* msg =
                QT_TR_NOOP("Failed to run terminal with script: Could "
                           "not create temporary script file.");
        WARN << msg << file.errorString();
        QMessageBox::warning(nullptr, {}, tr(msg) + " " + file.errorString());
    }

    else
    {
        // Note for future self
        // QTemporaryFile does not start
        // Deleting the file introduces race condition

        file.write("clear; ");
        file.write(s.toUtf8());
        file.close();

        auto command = QString("%1 -i %2").arg(pwd->pw_shell, file.fileName());

        runDetachedProcess({"/usr/bin/osascript", "-l", "AppleScript",
                            "-e", QString(apple_script_).arg(command)});
    }
}
