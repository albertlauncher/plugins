// Copyright (c) 2024 Manuel Schneider

#pragma once
#include <vector>
#include <map>
#include <QUrl>
#include <QString>
#include <albert/action.h>
#include <albert/export.h>
#include <albert/extension.h>
class QWidget;


namespace applications {


// class ALBERT_EXPORT Application
// {
// public:

//     /// The name of the application.
//     virtual QString name() const = 0;

//     /// The path to the application.
//     /// Note that this must not be the executable, but rather platform
//     /// specific application artefacts like bundles or desktop files.
//     virtual QString path() const = 0;

//     /// Launch this application.
//     /// The behavior when passing URLs is implementation specific. It may open
//     /// all URLs in one app instance, open an app instance per URL or ignore
//     /// them entirely. See Applications::mimeHandlers and
//     /// Applications::urlHandlers.
//     /// \param urls Optional URLs to open
//     virtual void run(const QUrl &url = {}) const = 0;

// protected:

//     virtual ~Application() = default;

// };


// class ALBERT_EXPORT Terminal : public Application
// {
// public:

//     /// Launch this terminal.
//     using Application::run;  // Prevent function hiding

// protected:

//     ~Terminal() override = default;

// };


class ALBERT_EXPORT Application
{
public:
    virtual QString id() const = 0;
    virtual QString name() const = 0;
    virtual QString path() const = 0;
    virtual void launch(const QString &working_dir = {}) const = 0;
    virtual void launchWithUrls(QStringList urls = {}, const QString &working_dir = {}) const = 0;
protected:
    virtual ~Application() = default;
};


class ALBERT_EXPORT Terminal : public Application
{
public:
    virtual void launchWithScript(QString script, const QString &working_dir = {}) const = 0;
protected:
    virtual ~Terminal() = default;
};


class ALBERT_EXPORT Plugin : virtual public albert::Extension
{
public:

    /// Launch a script in the user configured terminal and shell.
    ///
    /// To keep the terminal open use `exec $SHELL`
    ///
    /// \param script The script to run
    /// \param working_dir The working directory
    virtual void runTerminal(const QString &script = {}, const QString &working_dir = {}) const = 0;
    virtual std::shared_ptr<Terminal> userTerminal() const = 0;


protected:

    virtual ~Plugin() = default;

};


}
