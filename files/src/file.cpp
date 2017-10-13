// Copyright (C) 2014-2017 Manuel Schneider

#include "file.h"
#include <QApplication>
#include <QClipboard>
#include <QDesktopServices>
#include <QDir>
#include <QFileInfo>
#include <QMimeData>
#include <QProcess>
#include <QUrl>
#include "xdg/iconlookup.h"
#include "util/standardactions.h"
using namespace std;
using namespace Core;
extern QString terminalCommand;

map<QString,QString> Files::File::iconCache_;


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
#ifdef __linux__
    if ( result.startsWith(QDir::homePath()) )
        result.replace(QDir::homePath(), "~");
#endif
    return result;
}


/** ***************************************************************************/
QString Files::File::iconPath() const {

    const QString xdgIconName = mimetype().iconName();

    // First check if icon exists
    auto search = iconCache_.find(xdgIconName);
    if(search != iconCache_.end())
        return search->second;

    QString icon = XDG::IconLookup::iconPath({xdgIconName, mimetype().genericIconName(), "unknown"});
    if ( !icon.isEmpty() ) {
        iconCache_.emplace(xdgIconName, icon);
        return icon;
    }

    // Nothing found, return a fallback icon
    if ( xdgIconName == "inode-directory" ) {
        icon = ":directory";
        iconCache_.emplace(xdgIconName, icon);
    } else {
        icon = ":unknown";
        iconCache_.emplace(xdgIconName, icon);
    }
    return icon;
}


/** ***************************************************************************/
vector<shared_ptr<Action>> Files::File::actions() {

    vector<shared_ptr<Action>> actions;

    actions.push_back(make_shared<UrlAction>("Open with default application",
                                             QUrl::fromLocalFile(filePath())));

    QFileInfo fileInfo(filePath());
    if ( fileInfo.isFile() && fileInfo.isExecutable() )
        actions.push_back(make_shared<ProcAction>("Execute file", QStringList{filePath()}));


    actions.push_back(make_shared<UrlAction>("Reveal in file browser",
                                             QUrl::fromLocalFile(QFileInfo(filePath()).path())));


    actions.push_back(make_shared<FuncAction>("Open terminal at this path", [this](){
        QFileInfo fileInfo(filePath());
        QStringList commandLine = terminalCommand.trimmed().split(' ');
        if ( commandLine.size() == 0 )
            return;
        QProcess::startDetached(commandLine[0], {}, fileInfo.isDir() ? fileInfo.filePath() : fileInfo.path());
    }));

    actions.push_back(make_shared<FuncAction>("Copy file to clipboard", [this](){
        //  Get clipboard
        QClipboard *cb = QApplication::clipboard();

        // Ownership of the new data is transferred to the clipboard.
        QMimeData* newMimeData = new QMimeData();

        // Copy old mimedata
        const QMimeData* oldMimeData = cb->mimeData();
        for (const QString &f : oldMimeData->formats())
            newMimeData->setData(f, oldMimeData->data(f));

        // Copy path of file
        QString filePath = this->filePath();
        newMimeData->setText(filePath);

        // Copy file
        newMimeData->setUrls({QUrl::fromLocalFile(filePath)});

        // Copy file (f*** you gnome)
        QByteArray gnomeFormat = QByteArray("copy\n").append(QUrl::fromLocalFile(filePath).toEncoded());
        newMimeData->setData("x-special/gnome-copied-files", gnomeFormat);

        // Set the mimedata
        cb->setMimeData(newMimeData);
    }));

    actions.push_back(make_shared<ClipAction>("Copy path to clipboard", filePath()));

    return actions;
}



/** ***************************************************************************/
vector<IndexableItem::IndexString> Files::File::indexStrings() const {
    vector<IndexableItem::IndexString> res;
    res.emplace_back(name(), UINT_MAX);
    // TODO ADD PATH
    return res;
}
