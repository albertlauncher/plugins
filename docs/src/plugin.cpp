// Copyright (c) 2022-2024 Manuel Schneider

#include "configwidget.h"
#include "plugin.h"
#include <QDirIterator>
#include <QImageWriter>
#include <QJsonArray>
#include <QJsonParseError>
#include <QMessageBox>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QSqlDatabase>
#include <QTemporaryDir>
#include <albert/albert.h>
#include <albert/logging.h>
#include <archive.h>
#include <archive_entry.h>
ALBERT_LOGGING_CATEGORY("docs")
using namespace albert;
using namespace std;

static const char *docsets_dir = "docsets";
Plugin *Plugin::instance_ = nullptr;

static QString extract(const QString &src, const QString &dst)
{
    struct archive* a;
    a = archive_read_new();
    archive_read_support_format_all(a);
    archive_read_support_filter_all(a);

    QString err;

    if (int ret = archive_read_open_filename(a, src.toUtf8().constData(), 10240); ret == ARCHIVE_OK)
    {
        struct archive_entry* entry;
        int extract_flags = ARCHIVE_EXTRACT_TIME | ARCHIVE_EXTRACT_PERM | ARCHIVE_EXTRACT_ACL | ARCHIVE_EXTRACT_FFLAGS;
        while (true)
        {
            if (ret = archive_read_next_header(a, &entry); ret != ARCHIVE_OK)
            {
                if (ret != ARCHIVE_EOF) // elsefinsihed
                    err = QString("(%1) %2").arg(ret).arg(archive_error_string(a));
                break;
            }

            archive_entry_set_pathname(entry, QDir(dst).filePath(archive_entry_pathname(entry)).toLocal8Bit().constData());

            if (ret = archive_read_extract(a, entry, extract_flags); ret != ARCHIVE_OK)
            {
                err = QString("(%1) %2").arg(ret).arg(archive_error_string(a));
                break;
            }
        }

        archive_read_close(a);
    }
    else
        err = QString("(%1) %2").arg(ret).arg(archive_error_string(a));

    archive_read_free(a);

    return err;
}

static void saveBase64ImageToFile(const QByteArray& base64Data, const auto& filePath)
{
    QByteArray imageData = QByteArray::fromBase64(base64Data);
    QImage image;
    image.loadFromData(imageData);

    if (!image.isNull()) {
        QFile file(filePath);
        if (file.open(QIODevice::WriteOnly)) {
            QString fileFormat = QFileInfo(filePath).suffix().toLower();
            QImageWriter writer(&file, fileFormat.toUtf8());
            if (!writer.write(image))
                WARN << "Failed to save image";
            file.close();
        }
        else WARN << "Failed to open file for writing";
    }
    else WARN << "Failed to load image from Base64 data";
}


Plugin::Plugin()
{
    instance_ = this;

    if(!QSqlDatabase::isDriverAvailable("QSQLITE"))
        throw "QSQLITE driver unavailable";

    tryCreateDirectory(dataLocation() / docsets_dir);
    tryCreateDirectory(dataLocation() / "icons");

    connect(this, &Plugin::docsetsChanged, this, &Plugin::updateIndexItems);

    updateDocsetList();
}

Plugin::~Plugin()
{
    if (download_)
        cancelDownload();
}

Plugin *Plugin::instance() { return instance_; }

void Plugin::updateIndexItems()
{
    vector<IndexItem> items;

    for (const auto &docset : docsets_)
        if (!docset.path.isNull())
            docset.createIndexItems(items);

    setIndexItems(::move(items));
}

QWidget *Plugin::buildConfigWidget() { return new ConfigWidget; }

const vector<Docset> &Plugin::docsets() const { return docsets_; }

void Plugin::updateDocsetList()
{
    if (download_)
        return;

    static const char *url = "https://api.zealdocs.org/v1/docsets";

    debug(tr("Downloading docset list from '%1'").arg(url));

    QNetworkReply *reply = network().get(QNetworkRequest(QUrl{url}));
    reply->setParent(this); // For the case the plugin is deleted before the reply is finished

    connect(reply, &QNetworkReply::finished, this, [this, reply]
    {
        reply->deleteLater();

        QByteArray replyData;
        QFile cachedDocsetListFile(QDir(cacheLocation()).filePath("zeal_docset_list.json"));

        if (reply->error() != QNetworkReply::NoError)
        {
            if (cachedDocsetListFile.open(QIODevice::ReadOnly | QIODevice::Text))
            {
                replyData = cachedDocsetListFile.readAll();
                cachedDocsetListFile.close();
            }
            else
                return error(tr("Error fetching docset list: %1").arg(reply->errorString()));
        }
        else
            replyData = reply->readAll();

        docsets_.clear();

        QJsonParseError parse_error;
        const QJsonDocument json_document = QJsonDocument::fromJson(replyData, &parse_error);
        if (parse_error.error == QJsonParseError::NoError)
        {
            for (const QJsonValue &val : json_document.array())
            {
                QJsonObject obj = val.toObject();

                auto name = obj[QStringLiteral("name")].toString();
                auto title = obj[QStringLiteral("title")].toString();
                auto source = obj[QStringLiteral("sourceId")].toString();
                auto icon_path = QDir(cacheLocation()).filePath(QString("icons/%1.png").arg(name));
                auto rawBase64 = obj[QStringLiteral("icon2x")].toString().toLocal8Bit();
                saveBase64ImageToFile(rawBase64, icon_path);

                docsets_.emplace_back(name, title, source, icon_path);

                QDir dir(QString("%1/%2.docset").arg(QDir(dataLocation()).filePath(docsets_dir), name));
                if (dir.exists())
                    docsets_.back().path = dir.path();
            }
            debug(tr("Docset list updated."));

            if (reply->error() == QNetworkReply::NoError)
            {
                if (cachedDocsetListFile.open(QIODevice::WriteOnly | QIODevice::Text))
                {
                    cachedDocsetListFile.write(replyData);
                    cachedDocsetListFile.close();
                }
                else
                    debug(tr("Failed to save fetched docset list: %1").arg(cachedDocsetListFile.errorString()));
            }
        }
        else
            error(tr("Failed to parse docset list: %1").arg(parse_error.errorString()));

        emit docsetsChanged();
    });
}

void Plugin::downloadDocset(uint index)
{
    Q_ASSERT(!download_);

    auto &ds = docsets_.at(index);

    auto url = QUrl{QString("https://go.zealdocs.org/d/%1/%2/latest")
            .arg(ds.source_id.chopped(5), ds.name)};

    debug(tr("Downloading docset from '%1'").arg(url.toString()));
    download_ = network().get(QNetworkRequest(url));

    connect(download_, &QNetworkReply::downloadProgress,
            this, [this](qint64 bytesReceived, qint64 bytesTotal)
    {
        auto info = QString("%1/%2 MiB").arg((float)bytesReceived/1000000, 0, 'f', 1)
                                        .arg((float)bytesTotal/1000000, 0, 'f', 1);
        emit statusInfo(info);
    });

    connect(download_, &QNetworkReply::finished, this, [this, &ds] () mutable
    {
        if (download_)  // else aborted
        {
            auto tmp_dir = QTemporaryDir();
            if (tmp_dir.isValid())
            {
                // write downloaded data to file
                if (QFile file(tmp_dir.filePath(download_->url().fileName())); file.open(QIODevice::WriteOnly))
                {
                    while (download_->bytesAvailable())
                        file.write(download_->read(1000000));
                    file.close();

                    debug(tr("Extracting file '%1'").arg(file.fileName()));
                    auto cache_loc = QString(cacheLocation().c_str());
                    if (QString err = extract(file.fileName(), cache_loc); err.isEmpty())
                    {
                        debug(tr("Searching docset in '%1'").arg(cache_loc));
                        if (QDirIterator it(cache_loc, {"*.docset"}, QDir::Dirs, QDirIterator::Subdirectories); it.hasNext())
                        {
                            auto src = it.next();
                            auto dst = QString("%1/%2.docset").arg(QDir(dataLocation()).filePath(docsets_dir), ds.name);
                            debug(tr("Renaming '%1' to '%2'").arg(src, dst));
                            if (QFile::rename(src, dst))
                            {
                                ds.path = dst;
                                emit docsetsChanged();
                                updateIndexItems();
                                emit statusInfo(tr("Docset '%1' ready.").arg(ds.name));
                            }
                            else
                                error(tr("Failed renaming dir '%1' to '%2'.").arg(src, dst));
                        }
                        else
                            error(tr("Failed finding extracted docset in %1").arg(cache_loc));
                    }
                    else
                        error(tr("Extracting docset failed: '%1' (%2)").arg(file.fileName(), err));

                    QFile::remove(file.fileName());
                }
                else
                    error(tr("Failed to write to file: '%1'").arg(file.fileName()));
            }
            else
                error(tr("Failed creating temporary directory"));

            download_ = nullptr;
        }
        else
            debug(tr("Cancelled '%1' docset download.").arg(ds.name));

        emit downloadStateChanged();
    });

    // Delete reply in any case. may be cancelled.
    connect(download_, &QNetworkReply::finished, download_, &QNetworkReply::deleteLater);

    emit downloadStateChanged();
}

void Plugin::cancelDownload()
{
    Q_ASSERT(download_);
    auto dn = download_;
    download_ = nullptr;  // state aborted in finished()
    dn->abort(); // emits finished directly
}

bool Plugin::isDownloading() const { return download_; }

void Plugin::removeDocset(uint index)
{
    auto &ds = docsets_.at(index);

    if (!ds.isInstalled())
    {
        WARN << "Docset not installed";
    }
    else if (QDir dir(ds.path); !dir.exists())
    {
        WARN << "Docset dir does not exist";
        ds.path.clear();
        emit docsetsChanged();
    }
    else if (QMessageBox::question(nullptr, qApp->applicationName(),
                                   tr("Remove docset '%1' at %2?").arg(ds.title, ds.path))
             != QMessageBox::Yes)
    {
        DEBG << "Docset removal cancelled by user";
    }
    else if (!dir.removeRecursively())
    {
        // Note this may fail if filebrowser is open on macos
        error(tr("Failed to remove directory '%1'").arg(ds.path));
    }
    else
    {
        debug(tr("Directory removed '%1'").arg(ds.path));
        ds.path.clear();
        emit docsetsChanged();
    }
}

void Plugin::debug(const QString &msg)
{
    DEBG << msg;
    emit statusInfo(msg);
}

void Plugin::error(const QString &msg, QWidget * modal_parent)
{
    WARN << msg;
    emit statusInfo(msg);
    QMessageBox::warning(modal_parent, qApp->applicationDisplayName(), msg);
}
