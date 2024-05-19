// Copyright (c) 2022-2024 Manuel Schneider

#include "plugin.h"
#include <QDir>
#include <QDirIterator>
#include <QFileInfo>
#include <QImageWriter>
#include <QJsonArray>
#include <QJsonParseError>
#include <QLabel>
#include <QListWidget>
#include <QMessageBox>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QSqlDatabase>
#include <QSqlDriver>
#include <QSqlError>
#include <QSqlQuery>
#include <QTemporaryDir>
#include <QXmlStreamReader>
#include <albert/logging.h>
#include <albert/util.h>
#include <archive.h>
#include <archive_entry.h>
ALBERT_LOGGING_CATEGORY("docs")
using namespace albert;
using namespace std;

static const char *docsets_dir = "docsets";

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

static void saveBase64ImageToFile(const QByteArray& base64Data, const QString& filePath)
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

static QString fixPath(QString path) {
    static const QRegularExpression dashEntryRegExp(QStringLiteral("<dash_entry_.*>"));
    path.remove(dashEntryRegExp);
    return path;
}


class DocumentationItem : public albert::Item
{
public:
    DocumentationItem(const Docset *ds, const QString &n, const QString &p)
        : docset(ds), name(n), path(fixPath(p)) { }

    const Docset * const docset;
    const QString name;
    const QString path;

    QString id() const override
    { return path; }

    QString text() const override
    { return name; }

    QString subtext() const override
    { return docset->title; }

    QStringList iconUrls() const override
    { return {"file:"+docset->icon_path}; }

    QString inputActionText() const override
    { return name; }

    vector<Action> actions() const override
    {
        return {
            {
                id(), Plugin::tr("Open documentation"), [this] {
                    openUrl(QString("file://%1/Contents/Resources/Documents/%2")
                            .arg(docset->path, path));
                }
        }};
    }
};


Plugin::Plugin()
{
    if(!QSqlDatabase::isDriverAvailable("QSQLITE"))
        throw "QSQLITE driver unavailable";

    auto data_dir = createOrThrow(dataLocation());
    if (!data_dir.mkpath(docsets_dir))
        throw "Unable to create docsets dir";

    auto cache_dir = createOrThrow(cacheLocation());
    if (!cache_dir.mkpath("icons"))
        throw "Unable to create icons dir";

    connect(this, &Plugin::docsetsChanged, this, &Plugin::updateIndexItems);

    updateDocsetList();
}

Plugin::~Plugin()
{
    if (download_)
        cancelDownload();
}

void Plugin::updateIndexItems()
{
    vector<IndexItem> items;

    for (auto &[name, docset] : docsets_) {
        if (!docset.path.isNull()){
            auto docset_items = docset.createIndexItems();
            items.insert(items.end(),
                         std::make_move_iterator(docset_items.begin()),
                         std::make_move_iterator(docset_items.end()));
        }
    }

    setIndexItems(std::move(items));
}

QWidget *Plugin::buildConfigWidget() { return new ConfigWidget(this); }

const std::map<QString, Docset> &Plugin::docsets() const { return docsets_; }

void Plugin::updateDocsetList()
{
    static const char *url = "https://api.zealdocs.org/v1/docsets";
    debug(tr("Downloading docset list from '%1'").arg(url));
    QNetworkReply *reply = albert::network()->get(QNetworkRequest(QUrl{url}));
    reply->setParent(this); // For the case the plugin is deleted before the reply is finished

    connect(reply, &QNetworkReply::finished, this, [this, reply](){
        reply->deleteLater();

        QByteArray replyData;
        QFile cachedDocsetListFile(QDir(cacheLocation()).filePath("zeal_docset_list.json"));

        if (reply->error() != QNetworkReply::NoError) {
            if (cachedDocsetListFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
                replyData = cachedDocsetListFile.readAll();
                cachedDocsetListFile.close();
            } else
                return error(tr("Error fetching docset list: %1").arg(reply->errorString()));
        } else
            replyData = reply->readAll();

        docsets_.clear();

        QJsonParseError parse_error;
        const QJsonDocument json_document = QJsonDocument::fromJson(replyData, &parse_error);
        if (parse_error.error == QJsonParseError::NoError) {
            for (const QJsonValue &v : json_document.array()) {
                QJsonObject jsonObject = v.toObject();

                auto source = jsonObject[QStringLiteral("sourceId")].toString();
                auto identifier = jsonObject[QStringLiteral("name")].toString();
                auto title = jsonObject[QStringLiteral("title")].toString();
                auto icon_path = QDir(cacheLocation()).filePath(QString("icons/%1.png").arg(identifier));
//                auto path = data_dir->filePath(QString("%1/%2").arg(docsets_dir, identifier));

                auto rawBase64 = jsonObject[QStringLiteral("icon2x")].toString().toLocal8Bit();
                saveBase64ImageToFile(rawBase64, icon_path);

                auto [it, success] = docsets_.emplace(identifier, Docset{source, identifier, title, icon_path});

                QDir dir(QString("%1/%2.docset").arg(QDir(dataLocation()).filePath(docsets_dir), identifier));
                if (dir.exists())
                    it->second.path = dir.path();
            }
            debug(tr("Docset list updated."));

            if (reply->error() == QNetworkReply::NoError) {
                if (cachedDocsetListFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
                    cachedDocsetListFile.write(replyData);
                    cachedDocsetListFile.close();
                } else
                    debug(tr("Failed to save fetched docset list: %1").arg(cachedDocsetListFile.errorString()));
            }
        }
        else
            error(tr("Failed to parse docset list: %1").arg(parse_error.errorString()));

        emit docsetsChanged();
    });
}

void Plugin::downloadDocset(const QString &name)
{
    Q_ASSERT(!download_);

    auto *docset = &docsets_.at(name);
    static const char *docsets_url_template = "https://go.zealdocs.org/d/%1/%2/latest";
    auto url = QUrl{QString(docsets_url_template).arg(docset->source_id.chopped(5), docset->identifier)};
    debug(tr("Downloading docset from '%1'").arg(url.toString()));
    download_ = albert::network()->get(QNetworkRequest(url));

    connect(download_, &QNetworkReply::downloadProgress,
            this, [this](qint64 bytesReceived, qint64 bytesTotal){
        auto info = QString("%1/%2 MiB").arg((float)bytesReceived/1000000, 0, 'f', 1)
                                        .arg((float)bytesTotal/1000000, 0, 'f', 1);
        emit statusInfo(info);
    });

    connect(download_, &QNetworkReply::finished, this, [this, docset]()
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
                    if (QString err = extract(file.fileName(), cacheLocation()); err.isEmpty())
                    {
                        debug(tr("Searching docset in '%1'").arg(cacheLocation()));
                        if (QDirIterator it(cacheLocation(), {"*.docset"}, QDir::Dirs, QDirIterator::Subdirectories); it.hasNext())
                        {
                            auto src = it.next();
                            auto dst = QString("%1/%2.docset").arg(QDir(dataLocation()).filePath(docsets_dir), docset->identifier);
                            debug(tr("Renaming '%1' to '%2'").arg(src, dst));
                            if (QFile::rename(src, dst))
                            {
                                docset->path = dst;
                                emit docsetsChanged();
                                updateIndexItems();
                                emit statusInfo(tr("Docset '%1' ready.").arg(docset->identifier));
                            }
                            else
                                error(tr("Failed renaming dir '%1' to '%2'.").arg(src, dst));
                        }
                        else
                            error(tr("Failed finding extracted docset in %1").arg(cacheLocation()));
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
            debug(tr("Cancelled '%1' docset download.").arg(docset->identifier));

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

void Plugin::removeDocset(const QString &name)
{
    auto &docset = docsets_.at(name);
    QDir dir(docset.path);
    if (dir.exists() && dir.removeRecursively()){  // Note this may fail if filebrowser is open on macos
        debug(tr("Directory removed '%1'").arg(docset.path));
        docset.path.clear();
        emit docsetsChanged();
    }
    else
        error(tr("Failed to remove directory '%1'").arg(docset.path));
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

///////////////////////////////////////////////////////////////////////////////

Docset::Docset(QString sid, QString id, QString t, QString ip)
    : source_id(sid), identifier(id), title(t), icon_path(ip)
{

}

std::vector<IndexItem> Docset::createIndexItems() const
{
    std::vector<IndexItem> items;

    if (auto file_path = QString("%1/Contents/Resources/Tokens.xml").arg(path); QFile::exists(file_path))
    {
        INFO << "Indexing docset" << file_path;

        QFile f(file_path);
        if (!f.open(QIODevice::ReadOnly|QIODevice::Text))
        {
            WARN << f.errorString();
            return {};
        }

        QXmlStreamReader xml(&f);
        xml.readNext();

        while (!xml.atEnd() && !xml.hasError())
        {
            auto tokenType = xml.readNext();
            if (tokenType == QXmlStreamReader::StartElement && xml.name() == QLatin1String("Token"))
            {
                QString n, t, p, a;
                for (;!(xml.tokenType() == QXmlStreamReader::EndElement && xml.name() == QLatin1String("Token")); xml.readNext())
                {
                    if (xml.tokenType() == QXmlStreamReader::StartElement)
                    {
                        if (xml.name() == QLatin1String("TokenIdentifier"))
                        {
                            for (;!(xml.tokenType() == QXmlStreamReader::EndElement && xml.name() == QLatin1String("TokenIdentifier")); xml.readNext())
                            {
                                if (xml.name() == QLatin1String("Name"))
                                    n = xml.readElementText();
                                else if (xml.name() == QLatin1String("Type"))
                                    t = xml.readElementText();
                            }
                        }
                        else if (xml.name() == QLatin1String("Path"))
                            p = xml.readElementText();
                        else if (xml.name() == QLatin1String("Anchor"))
                            a = xml.readElementText();
                    }
                }
                if (!a.isEmpty())
                    continue; // Skip anchors, browsers cant handle them

                auto item = make_shared<DocumentationItem>(this, n, p);
                items.emplace_back(item, item->text());
            }
        }
        f.close();
    }
    else if (file_path = QString("%1/Contents/Resources/docSet.dsidx").arg(path); QFile::exists(file_path))
    {
        INFO << "Indexing docset" << file_path;

        {
            QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", docsets_dir);
            db.setDatabaseName(file_path);
            if (!db.open())
            {
                WARN << "Unable to open database connection" << db.databaseName();
                return {};
            }

            QSqlQuery sql(db);

            // Check docset type
            if (!sql.exec("SELECT name FROM sqlite_master WHERE type='table' AND name='searchIndex'"))
            {
                WARN << sql.lastQuery() << sql.lastError().text();
                return {};
            }

            if (sql.next()) // returns true if searchIndex exists
            {
                if (!sql.exec("SELECT name, type, path FROM searchIndex ORDER BY name;"))
                {
                    WARN << sql.lastQuery() << sql.lastError().text();
                    return {};
                }

                while (sql.next())
                {
                    QString n = sql.value(0).toString();
                    //QString t = sql.value(1).toString();
                    QString p = sql.value(2).toString();
                    if (p.contains("#"))
                        continue; // Skip anchors, browsers cant handle them

                    auto item = make_shared<DocumentationItem>(this, n, p);
                    items.emplace_back(item, item->text());
                }
            }
            else
            {
                if(!sql.exec(R"R(
                    SELECT
                        ztokenname, ztypename, zpath, zanchor
                    FROM ztoken
                        INNER JOIN ztokenmetainformation ON ztoken.zmetainformation = ztokenmetainformation.z_pk
                        INNER JOIN zfilepath ON ztokenmetainformation.zfile = zfilepath.z_pk
                        INNER JOIN ztokentype ON ztoken.ztokentype = ztokentype.z_pk
                    ORDER BY ztokenname;
                )R"))
                {
                    WARN << sql.lastQuery() << sql.lastError().text();
                    return {};
                }

                while (sql.next()){
                    QString n = sql.value(0).toString();
                    //QString t = sql.value(1).toString();
                    QString p = sql.value(2).toString();
                    QString a = sql.value(3).toString();
                    if (!a.isEmpty())
                        continue; // Skip anchors, browsers cant handle them

                    auto item = make_shared<DocumentationItem>(this, n, p);
                    items.emplace_back(item, item->text());
                }
            }
            db.close();
        }
        QSqlDatabase::removeDatabase(docsets_dir);
    }
    return items;
}

///////////////////////////////////////////////////////////////////////////////

ConfigWidget::ConfigWidget(Plugin *p) : plugin(p)
{
    ui.setupUi(this);

    connect(ui.update_button, &QPushButton::pressed, plugin, &Plugin::updateDocsetList);

    connect(ui.cancel_button, &QPushButton::pressed, plugin, &Plugin::cancelDownload);

    connect(plugin, &Plugin::docsetsChanged, this, &ConfigWidget::updateDocsets);

    connect(plugin, &Plugin::statusInfo, ui.status_label, &QLabel::setText);

    connect(plugin, &Plugin::downloadStateChanged, this, [this](){
        ui.cancel_button->setVisible(plugin->isDownloading());
        ui.list_widget->setEnabled(!plugin->isDownloading());
        ui.update_button->setEnabled(!plugin->isDownloading());
    });

    connect(ui.list_widget, &QListWidget::itemChanged, this, [this](QListWidgetItem* item){
        auto &ds = plugin->docsets().at(item->data(Qt::UserRole).toString());
        if (item->checkState() == Qt::Checked)
            plugin->downloadDocset(ds.identifier);
        else {
            auto ret = QMessageBox::question(this, qApp->applicationName(),
                                             tr("Remove docset '%1' at %2?").arg(ds.title, ds.path));
            if (ret == QMessageBox::Yes)
                plugin->removeDocset(ds.identifier);
        }
        updateDocsets();
    });

    ui.cancel_button->hide();
    updateDocsets();
}

void ConfigWidget::updateDocsets()
{
    ui.list_widget->clear();
    QSignalBlocker blocker(this); // avoid itemChanged being triggered
    for (const auto &[name, docset] : plugin->docsets()){
        auto *item = new QListWidgetItem();
        item->setText(docset.title);
        item->setIcon(QIcon(docset.icon_path));
        item->setData(Qt::ToolTipRole, QString("%1 %2").arg(docset.identifier, docset.path));
        item->setData(Qt::UserRole, docset.identifier);
        item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
        item->setCheckState(docset.path.isNull() ? Qt::Unchecked : Qt::Checked);
        ui.list_widget->addItem(item);
    }
}
