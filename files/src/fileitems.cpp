// Copyright (c) 2022-2024 Manuel Schneider

#include "albert/albert.h"
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

QString FileItem::id() const { return filePath(); }

QString FileItem::text() const { return name(); }

QString FileItem::subtext() const { return filePath(); }

QString FileItem::inputActionText() const
{
    const QString &path = filePath();
    QString result = (QFileInfo(path).isDir()) ? QString("%1/").arg(path) : path;
#ifdef Q_OS_UNIX
    if (result.startsWith(QDir::homePath()))
        result.replace(QDir::homePath(), "~");
#endif
    return result;
}

QStringList FileItem::iconUrls() const
{
    QStringList urls;
#if defined(Q_OS_LINUX) || defined(Q_OS_FREEBSD)
    urls << QString("xdg:%1").arg(mimeType().iconName());
    urls << QString("xdg:%1").arg(mimeType().genericIconName());
#endif
    urls << QString("qfip:%1").arg(filePath());
    return urls;
}

vector<Action> FileItem::actions() const
{
    vector<Action> actions;

    static const auto tr_o = QCoreApplication::translate("FileItem", "Open with default application");
    actions.emplace_back(
        "f-open", tr_o,
        [this]()
        {
            openUrl(QUrl::fromLocalFile(filePath()).toString());
        });

    if (QFileInfo fi(filePath()); fi.isFile() && fi.isExecutable())
    {
        static const auto tr_e = QCoreApplication::translate("FileItem", "Execute");
        actions.emplace_back(
            "f-exec", tr_e,
            [this]()
            {
                runDetachedProcess({filePath()});
            });
    }

    static const auto tr_r = QCoreApplication::translate("FileItem", "Reveal in file browser");
    actions.emplace_back(
        "f-reveal", tr_r,
        [this]()
        {
            openUrl(QUrl::fromLocalFile(QFileInfo(filePath()).path()).toString());
        });

    static const auto tr_t = QCoreApplication::translate("FileItem", "Open terminal here");
    actions.emplace_back(
        "f-term", tr_t,
        [this]()
        {
            QFileInfo fi(filePath());
            runTerminal({}, fi.isDir() ? fi.filePath() : fi.path());
        });


    static const auto tr_c = QCoreApplication::translate("FileItem", "Copy file to clipboard");
    actions.emplace_back(
        "f-copy", tr_c, [this]()
        {
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

    static const auto tr_cp = QCoreApplication::translate("FileItem", "Copy path to clipboard");
    actions.emplace_back(
        "f-copypath", tr_cp,
        [this](){
            setClipboardText(filePath());
        });

    return actions;
}


IndexFileItem::IndexFileItem(const QString &name, const QMimeType &mime, const std::shared_ptr<DirNode> &parent):
        name_(name), mimetype_(mime), parent_(parent) {}

QString IndexFileItem::name() const
{ return name_; }

QString IndexFileItem::path() const
{ return parent_->filePath(); }

QString IndexFileItem::filePath() const
{ return QString("%1/%2").arg(parent_->filePath(), name_); }

const QMimeType &IndexFileItem::mimeType() const
{ return mimetype_; }


StandardFile::StandardFile(QString path, QMimeType mimetype, QString completion)
        : completion_(::move(completion)), mimetype_(::move(mimetype))
{
    QFileInfo fileInfo(path);
    name_ = fileInfo.fileName();
    path_ = fileInfo.canonicalPath();
}

QString StandardFile::name() const
{ return name_; }

QString StandardFile::path() const
{ return path_; }

QString StandardFile::filePath() const
{ return QDir(path_).filePath(name_); }

const QMimeType &StandardFile::mimeType() const
{ return mimetype_; }

QString StandardFile::inputActionText() const
{ return completion_.isEmpty() ? FileItem::inputActionText() : completion_; }
