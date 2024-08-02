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


class ALBERT_EXPORT Applications : virtual public albert::Extension
{
public:

    /// Launch a script in the user shell the user configured terminal.
    ///
    /// To allow flexible closing logic exit use the exit builtin. E.g.
    /// `command && exit` with `close_on_exit = true` (close on succes).
    ///
    /// \param script The script to run
    /// \param working_dir The working directory
    /// \param close_on_exit Close the terminal on exit. Has no effect if script is emtpy.
    virtual void runTerminal(const QString &script = {}, const QString &working_dir = {}, bool close_on_exit = false) = 0;

    virtual void runTerminal(const QStringList &commandline, const QString &working_dir = {}) = 0;

    /// Create actions The applications on this system.
    virtual std::vector<albert::Action> actions(const QUrl&) const = 0;

protected:

    virtual ~Applications() = default;

};


}
