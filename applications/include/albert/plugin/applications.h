// Copyright (c) 2024 Manuel Schneider

#pragma once
#include <QStringList>
#include <QUrl>
#include <albert/export.h>
#include <albert/extension.h>
class QWidget;

namespace applications {

class ALBERT_EXPORT Application
{
public:

    /// The unique id
    /// \returns \copybrief
    virtual QString id() const = 0;

    /// The localized name
    /// \returns \copybrief
    virtual QString name() const = 0;

    /// The path to the spec, bundle or such
    /// \returns \copybrief
    virtual QString path() const = 0;

    /// Launch the application
    virtual void launch() const = 0;

    // /// The supported URL schemes
    // /// \sa launch(const QList<QUrl>&)
    // virtual QStringList schemes() const = 0;

    // /// Launch with URLs
    // /// \sa schemes
    // virtual void launch(const QList<QUrl> &urls) const = 0;

    // /// The supported mime types
    // /// \sa launch(const QStringList&)
    // virtual QStringList mimeTypes() const = 0;

    // /// Launch with files
    // /// \sa mimeTypes
    // virtual void launch(const QStringList &paths) const = 0;

protected:

    virtual ~Application() = default;

};


class ALBERT_EXPORT Plugin : virtual public albert::Extension
{
public:

    /// Launch a shell script in the users terminal and shell
    ///
    /// To keep the terminal open use `exec $SHELL`
    /// To set the working directory use `cd <working_dir>`
    ///
    /// Although the script is run in the users shell use
    /// Bourne Shell (`sh`) features only for compatibilty
    ///
    /// \param script The script to run
    virtual void runTerminal(const QString &script) const = 0;

protected:

    virtual ~Plugin() = default;

};

}
