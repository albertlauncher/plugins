// Copyright (C) 2014-2018 Manuel Schneider

#include <QApplication>
#include <QClipboard>
#include <QDesktopServices>
#include <QDir>
#include <QFileInfo>
#include <QMimeData>
#include <QProcess>
#include <QUrl>
#include <pwd.h>
#include <unistd.h>
#include "albert/util/standardactions.h"
#include "file.h"
#include "xdg/iconlookup.h"
using namespace std;
using namespace Core;


/** ***************************************************************************/
QString Files::File::id() const {
    return filePath();
}


/** ***************************************************************************/
QString Files::File::text() const {
    return name();
}


/** ***************************************************************************/
QString Files::File::subtext() const {
    return path();
}


/** ***************************************************************************/
QString Files::File::completion() const {
    const QString &path = filePath();
    QString result = ( QFileInfo(path).isDir() ) ? QString("%1/").arg(path) : path;
#ifdef __unix__
    if ( result.startsWith(QDir::homePath()) )
        result.replace(QDir::homePath(), "~");
#endif
    return result;
}


/** ***************************************************************************/
QString Files::File::iconPath() const {
    QString icon = XDG::IconLookup::iconPath({mimetype().iconName(), mimetype().genericIconName(), "unknown"});
    if ( !icon.isEmpty() )
        return icon;

    // Nothing found, return a fallback icon
    return (mimetype().iconName() == "inode-directory") ? ":directory" : ":unknown";
}


/** ***************************************************************************/
vector<shared_ptr<Action>> Files::File::actions() {
    return buildFileActions(filePath());
}

/** ***************************************************************************/
std::vector<std::shared_ptr<Action> > Files::File::buildFileActions(const QString &filePath)
{
    vector<shared_ptr<Action>> actions;

    actions.push_back(make_shared<UrlAction>("Open with default application",
                                             QUrl::fromLocalFile(filePath)));

    QFileInfo fileInfo(filePath);

    if ( fileInfo.isFile() && fileInfo.isExecutable() )
        actions.push_back(make_shared<ProcAction>("Execute file", QStringList{filePath}));


    actions.push_back(make_shared<UrlAction>("Reveal in file browser",
                                             QUrl::fromLocalFile(fileInfo.path())));

    // Get the user shell (passwd must not be freed)
    passwd *pwd = getpwuid(geteuid());
    if (pwd == nullptr)
        throw "Could not retrieve user shell";

    // Let standard shell handle flow control (syntax differs in shells, e.g. fish)
    actions.push_back(make_shared<TermAction>("Open terminal here", QStringList{pwd->pw_shell},
                                              fileInfo.isDir() ? fileInfo.filePath() : fileInfo.path()));

    actions.push_back(make_shared<FuncAction>("Copy file to clipboard", [filePath](){

        //  Get clipboard
        QClipboard *cb = QApplication::clipboard();

        // Ownership of the new data is transferred to the clipboard.
        QMimeData* newMimeData = new QMimeData();

        // Copy old mimedata
        const QMimeData* oldMimeData = cb->mimeData();
        for (const QString &f : oldMimeData->formats())
            newMimeData->setData(f, oldMimeData->data(f));

        // Copy path of file
        newMimeData->setText(filePath);

        // Copy file
        newMimeData->setUrls({QUrl::fromLocalFile(filePath)});

        // Copy file (f*** you gnome)
        QByteArray gnomeFormat = QByteArray("copy\n").append(QUrl::fromLocalFile(filePath).toEncoded());
        newMimeData->setData("x-special/gnome-copied-files", gnomeFormat);

        // Set the mimedata
        cb->setMimeData(newMimeData);
    }));

    actions.push_back(make_shared<ClipAction>("Copy path to clipboard", filePath));

    return actions;

}

/** ***************************************************************************/
vector<IndexableItem::IndexString> Files::File::indexStrings() const {
    vector<IndexableItem::IndexString> res;
    res.emplace_back(name(), UINT_MAX);
    // TODO ADD PATH
    return res;
}
