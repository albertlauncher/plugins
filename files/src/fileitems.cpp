// Copyright (c) 2022 Manuel Schneider

#include "albert.h"
#include "fileitems.h"
#include "fsindexnodes.h"
#include <QApplication>
#include <QClipboard>
#include <QDir>
#include <QFileInfo>
#include <QMimeData>
#include <QUrl>
using namespace albert;
using namespace std;

QString AbstractFileItem::id() const { return filePath(); }

QString AbstractFileItem::text() const { return name(); }

QString AbstractFileItem::subtext() const { return path(); }

QString AbstractFileItem::inputActionText() const
{
    const QString &path = filePath();
    QString result = (QFileInfo(path).isDir()) ? QString("%1/").arg(path) : path;
#ifdef Q_OS_UNIX
    if (result.startsWith(QDir::homePath()))
        result.replace(QDir::homePath(), "~");
#endif
    return result;
}

QStringList AbstractFileItem::iconUrls() const
{
    QStringList urls;
//    if (mimeType().name ().startsWith("image"))
//        urls << filePath();
//#ifdef Q_OS_LINUX
    urls << QString("xdg:%1").arg(mimeType().iconName());
    urls << QString("xdg:%1").arg(mimeType().genericIconName());
//#endif
    urls << QString("qfip:%1").arg(filePath());
    return urls;
}

Actions AbstractFileItem::actions() const
{
    albert::Actions actions;

    actions.emplace_back("f-open", "Open with default application", [this](){
        openUrl(QUrl::fromLocalFile(filePath()).toString());
    });

    if (QFileInfo fi(filePath()); fi.isFile() && fi.isExecutable())
        actions.emplace_back("f-exec", "Execute", [this](){
            runDetachedProcess({filePath()});
        });

    actions.emplace_back("f-reveal", "Reveal in file browser", [this](){
        QFileInfo fi(filePath());
        openUrl(QUrl::fromLocalFile(fi.path()).toString());
    });

    actions.emplace_back("f-term", "Open terminal here", [this](){
        QFileInfo fi(filePath());
        runTerminal({}, fi.isDir() ? fi.filePath() : fi.path());
    });

    actions.emplace_back("f-copy", "Copy file to clipboard", [this](){
        //  Get clipboard
        QClipboard *cb = QApplication::clipboard();

        // Ownership of the new data is transferred to the clipboard.
        auto *newMimeData = new QMimeData();

        // Copy old mimedata
        const QMimeData* oldMimeData = cb->mimeData();
        for (const QString &f : oldMimeData->formats())
            newMimeData->setData(f, oldMimeData->data(f));

        // Copy path of file
        newMimeData->setText(filePath());

        // Copy file
        newMimeData->setUrls({QUrl::fromLocalFile(filePath())});

        // Copy file (f*** you gnome)
        QByteArray gnomeFormat = QByteArray("copy\n").append(QUrl::fromLocalFile(filePath()).toEncoded());
        newMimeData->setData("x-special/gnome-copied-files", gnomeFormat);

        // Set the mimedata
        cb->setMimeData(newMimeData);
    });

    actions.emplace_back("f-clippath", "Copy path to clipboard", [this](){
        setClipboardText(filePath());
    });

    return actions;
}


IndexFileItem::IndexFileItem(const QString &name, const QMimeType &mime, const std::shared_ptr<DirNode> &parent):
        name_(name), mimetype_(mime), parent_(parent) {}

QString IndexFileItem::name() const { return name_; }

QString IndexFileItem::path() const { return parent_->filePath(); }

QString IndexFileItem::filePath() const { return QString("%1/%2").arg(parent_->filePath(), name_); }

const QMimeType &IndexFileItem::mimeType() const { return mimetype_; }


StandardFile::StandardFile(QString path, QMimeType mimetype, QString completion)
        : completion_(::move(completion)), mimetype_(::move(mimetype))
{
    QFileInfo fileInfo(path);
    name_ = fileInfo.fileName();
    path_ = fileInfo.canonicalPath();
}

QString StandardFile::name() const { return name_; }

QString StandardFile::path() const { return path_; }

QString StandardFile::filePath() const { return QDir(path_).filePath(name_); }

const QMimeType &StandardFile::mimeType() const { return mimetype_; }

QString StandardFile::inputActionText() const
{
    return completion_.isEmpty() ? AbstractFileItem::inputActionText() : completion_;
}
