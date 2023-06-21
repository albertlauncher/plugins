// Copyright (c) 2022 Manuel Schneider

#include "plugin.h"
#include <QImageWriter>
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
#include <QTemporaryFile>
#include <archive.h>
#include <archive_entry.h>
ALBERT_LOGGING
using namespace std;
using namespace albert;


static const char *docsets_dir = "docsets";

static bool extract(const QString &src, const QString &dst)
{
    struct archive* a;
    struct archive_entry* entry;
    int flags;
    int result = ARCHIVE_OK;

    a = archive_read_new();
    archive_read_support_format_all(a);
    archive_read_support_filter_all(a);

    if (archive_read_open_filename(a, src.toUtf8().constData(), 10240) != ARCHIVE_OK) {
        WARN << "Failed to open source archive: " << archive_error_string(a);
        return false;
    }

    if (chdir(dst.toUtf8().constData()) != 0) {
        WARN << "Failed to change directory: " << strerror(errno);
        archive_read_close(a);
        archive_read_free(a);
        return false;
    }

    while (archive_read_next_header(a, &entry) == ARCHIVE_OK) {
        flags = ARCHIVE_EXTRACT_TIME | ARCHIVE_EXTRACT_PERM | ARCHIVE_EXTRACT_ACL | ARCHIVE_EXTRACT_FFLAGS;
        result = archive_read_extract(a, entry, flags);
        if (result != ARCHIVE_OK) {
            WARN << "Extraction failed: " << archive_error_string(a);
            break;
        }
    }

    archive_read_close(a);
    archive_read_free(a);

    return !result;
}

static void saveBase64ImageToFile(const QByteArray& base64Data, const QString& filePath)
{
    QByteArray imageData = QByteArray::fromBase64(base64Data);
    QImage image;
    image.loadFromData(imageData);
    image = image.scaled(image.size()*2); // fix pixmap sizes

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
    DocumentationItem(const Docset *docset, QString name, QString path)
        : docset(docset), name(::move(name)), path(fixPath(path)) {}

    const Docset * const docset;
    const QString name;
    const QString path;

    QString id() const override { return path; }
    QString text() const override { return name; }
    QString subtext() const override
    {
        return QString("%1 documentation").arg(docset->identifier);
    }
    QStringList iconUrls() const override { return {docset->icon_path}; }
    QString inputActionText() const override { return name; }
    std::vector<Action> actions() const override
    {
        return {
            {
                id(),
                "Open documentation",
                [this](){
                    openUrl(QString("file://%1/Contents/Resources/Documents/%2").arg(docset->path, path));
                }
            }
        };
    }
};


Plugin::Plugin()
{
    if(!QSqlDatabase::isDriverAvailable("QSQLITE"))
        throw "QSQLITE driver unavailable";

    if (!dataDir().mkpath(docsets_dir))
        throw "Unable to create docsets dir";

    if (!cacheDir().mkpath("icons"))
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
    static const char *docsets_list_url = "https://api.zealdocs.org/v1/docsets";
    QNetworkReply *reply = albert::networkManager()->get(QNetworkRequest(QUrl{docsets_list_url}));
    connect(reply, &QNetworkReply::finished, this, [this, reply](){
        reply->deleteLater();

        if (reply->error() != QNetworkReply::NoError) {
            auto msg = QString("Error fetching docset list: %1").arg(reply->errorString());
            WARN << msg;
            emit statusInfo(msg);
            return;
        }

        docsets_.clear();

        QJsonParseError parse_error;
        const QJsonDocument json_document = QJsonDocument::fromJson(reply->readAll(), &parse_error);
        if (parse_error.error == QJsonParseError::NoError) {
            for (const QJsonValue &v : json_document.array()) {
                QJsonObject jsonObject = v.toObject();

                auto source = jsonObject[QStringLiteral("sourceId")].toString();
                auto identifier = jsonObject[QStringLiteral("name")].toString();
                auto title = jsonObject[QStringLiteral("title")].toString();
                auto icon_path = cacheDir().filePath(QString("icons/%1.png").arg(identifier));
                auto path = dataDir().filePath(QString("%1/%2").arg(docsets_dir, identifier));

                auto rawBase64 = jsonObject[QStringLiteral("icon2x")].toString().toLocal8Bit();
                saveBase64ImageToFile(rawBase64, icon_path);

                auto [it, success] = docsets_.emplace(identifier, Docset{source, identifier, title, icon_path});

                QDir dir(QString("%1/%2.docset").arg(dataDir().filePath(docsets_dir), identifier));
                if (dir.exists())
                    it->second.path = dir.path();


            }
        }
        else
            WARN << "Failed to parse docset list." << parse_error.errorString();

        emit docsetsChanged();
    });
}

void Plugin::downloadDocset(const QString &name)
{
    Q_ASSERT(!download_);

    auto *docset = &docsets_.at(name);
    static const char *docsets_url_template = "https://go.zealdocs.org/d/%1/%2/latest";
    auto url = QUrl{QString(docsets_url_template).arg(docset->source_id, docset->identifier)};
    DEBG << "Downloading" << url;
    download_ = albert::networkManager()->get(QNetworkRequest(url));

    connect(download_, &QNetworkReply::downloadProgress,
            this, [this](qint64 bytesReceived, qint64 bytesTotal){
        auto info = QString("%1/%2 MiB").arg((float)bytesReceived/1000000, 0, 'f', 1)
                                        .arg((float)bytesTotal/1000000, 0, 'f', 1);
        emit statusInfo(info);
    });

    connect(download_, &QNetworkReply::finished, this, [this, docset](){
        if (download_){  // else aborted

            QTemporaryDir tmp_dir;

            // write to file
            QFile tmp(tmp_dir.filePath(download_->url().fileName()));
            if (!tmp.open(QIODevice::WriteOnly))
                qFatal("Failed to open the file");
            while (download_->bytesAvailable())
                tmp.write(download_->read(1000000));
            tmp.close();

            // delete reply
            download_->deleteLater();
            download_ = nullptr;

            // extract temp file
            DEBG << "Extracting" << tmp.fileName();
            emit statusInfo("Extracting…");
            if (!extract(tmp.fileName(), tmp_dir.path())){
                DEBG << "Extracting docset failed" << tmp.fileName();
                emit statusInfo("Extracting docset failed.");
            } else {
                // rename the extracted dir to the identifier
                QDirIterator it(tmp_dir.path(), {"*.docset"}, QDir::Dirs, QDirIterator::Subdirectories);
                if (!it.hasNext()){
                    DEBG << "Failed finding extracted docset" << tmp.fileName();
                    emit statusInfo("Failed finding extracted docset.");
                } else {
                    auto extract_path = it.next();
                    auto target_path = QString("%1/%2.docset").arg(dataDir().filePath(docsets_dir), docset->identifier);
                    if (!QDir().rename(extract_path, target_path))
                        qFatal("Failed renaming dir");

                    docset->path = target_path;

                    emit statusInfo("Download finished.");
                    updateIndexItems();
                }
            }

            emit docsetsChanged();

            QFile::remove(tmp.fileName());
        } else {
            DEBG << "Downloading docset cancelled:" << docset->identifier;
            emit statusInfo("Download cancelled.");
        }

        emit downloadStateChanged();
    });

    emit downloadStateChanged();
}

void Plugin::cancelDownload()
{
    Q_ASSERT(download_);

//    QEventLoop loop;
//    connect(download_, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    auto d = download_;
    download_->deleteLater();
    download_ = nullptr;  // state aborted in finished()
    d->abort(); // emits finished directly
//    loop.exec(); // wait for finished
}

bool Plugin::isDownloading() const { return download_; }

void Plugin::removeDocset(const QString &name)
{
    auto &docset = docsets_.at(name);
    QDir dir(docset.path);
    if (dir.exists() && dir.removeRecursively()){  // Note this may fail if filebrowser is open on macos
        DEBG << "Directory removed" << docset.path;
        docset.path.clear();
        updateDocsetList();
    }
    else
        CRIT << "Failed to remove directory" << docset.path;

}

///////////////////////////////////////////////////////////////////////////////

Docset::Docset(QString source_id, QString identifier, QString title, QString icon_path)
    : source_id(source_id), identifier(identifier), title(title), icon_path(icon_path)
{

}

std::vector<IndexItem> Docset::createIndexItems() const
{
    std::vector<IndexItem> items;

    if (auto file_path = QString("%1/Contents/Resources/Tokens.xml").arg(path); QFile::exists(file_path)){

        QFile f(file_path);
        if (!f.open(QIODevice::ReadOnly|QIODevice::Text))
            return items;

        QXmlStreamReader xml(&f);
        xml.readNext();

        while (!xml.atEnd() && !xml.hasError())
        {
            auto tokenType = xml.readNext();

            if (tokenType == QXmlStreamReader::StartElement && xml.name() == QLatin1String("Token")) {
                QString n, t, p, a;

                for (;!(xml.tokenType() == QXmlStreamReader::EndElement && xml.name() == QLatin1String("Token")); xml.readNext()){

                    if (xml.tokenType() == QXmlStreamReader::StartElement){
                        if (xml.name() == QLatin1String("TokenIdentifier")){

                            for (;!(xml.tokenType() == QXmlStreamReader::EndElement && xml.name() == QLatin1String("TokenIdentifier")); xml.readNext()){
                                if (xml.name() == QLatin1String("Name"))
                                    n = xml.readElementText();
                                else if (xml.name() == QLatin1String("Type"))
                                    t = xml.readElementText();
                            }

                        } else if (xml.name() == QLatin1String("Path"))
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

    } else if (file_path = QString("%1/Contents/Resources/docSet.dsidx").arg(path); QFile::exists(file_path)) {

        {
            QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", docsets_dir);
            db.setDatabaseName(file_path);
            if (!db.open()){
                WARN << db.databaseName() << "Unable to open database connection.";
                return items;
            }

            QSqlQuery sql(db);

            // Check docset type
            sql.exec("SELECT name FROM sqlite_master WHERE type='table' AND name='{searchIndex}'");

            if (sql.size()) {  // searchIndex exists

                sql.exec("SELECT name, type, path FROM searchIndex ORDER BY name;");
                if (!sql.isActive())
                    qFatal("SQL ERROR: %s %s", qPrintable(sql.executedQuery()), qPrintable(sql.lastError().text()));
                while (sql.next()){
                    QString n = sql.value(0).toString();
                    QString t = sql.value(1).toString();
                    QString p = sql.value(2).toString();
                    if (p.contains("#"))
                        continue; // Skip anchors, browsers cant handle them

                    auto item = make_shared<DocumentationItem>(this, n, p);
                    items.emplace_back(item, item->text());
                }

            } else {

                sql.exec(R"R(
                    SELECT
                        ztokenname, ztypename, zpath, zanchor
                    FROM ztoken
                        INNER JOIN ztokenmetainformation ON ztoken.zmetainformation = ztokenmetainformation.z_pk
                        INNER JOIN zfilepath ON ztokenmetainformation.zfile = zfilepath.z_pk
                        INNER JOIN ztokentype ON ztoken.ztokentype = ztokentype.z_pk
                    ORDER BY ztokenname;
                )R");
                if (!sql.isActive())
                    qFatal("SQL ERROR: %s %s", qPrintable(sql.executedQuery()), qPrintable(sql.lastError().text()));
                while (sql.next()){
                    QString n = sql.value(0).toString();
                    QString t = sql.value(1).toString();
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
