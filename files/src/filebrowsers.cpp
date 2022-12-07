// Copyright (c) 2022 Manuel Schneider

#include "filebrowsers.h"
#include "fileitems.h"
using namespace std;

QString AbstractBrowser::description() const { return "Browse files by path"; }

bool AbstractBrowser::allowTriggerRemap() const { return false; }

static vector<shared_ptr<albert::Item>> buildItems(const QString &input)
{
    vector<shared_ptr<albert::Item>> results;

    // Get all matching files
    QFileInfo queryFileInfo(input);
    QFileInfo fi(queryFileInfo.path());
    if (fi.exists() && fi.isDir()) {
        QDir dir(fi.filePath());
        QString commonPrefix;
        QString queryFileName = queryFileInfo.fileName();
        QFileInfoList result_files;

        for (const QFileInfo &fileInfo: dir.entryInfoList(QDir::AllEntries | QDir::Hidden | QDir::NoDotAndDotDot,
                                                          QDir::DirsFirst | QDir::Name | QDir::IgnoreCase)) {
            QString fileName = fileInfo.fileName();

            if (fileName.startsWith(queryFileName)) {

                if (fileInfo.isDir())
                    fileName.append(QDir::separator());

                if (commonPrefix.isNull())
                    commonPrefix = fileName;
                else {
                    auto pair = mismatch(commonPrefix.begin(), commonPrefix.end(),
                                         fileName.begin(), fileName.end());
                    commonPrefix.resize(distance(commonPrefix.begin(), pair.first));
                }

                result_files << fileInfo;
            }
        }

        QMimeDatabase mimeDatabase;
        for (auto &rfi : result_files){
            QMimeType mimetype = mimeDatabase.mimeTypeForFile(rfi);

            results.emplace_back(make_shared<StandardFile>(
                    rfi.filePath(),
                    mimetype,
                    commonPrefix.isEmpty() ? commonPrefix : dir.filePath(commonPrefix)));
        }
    }
    return results;
}




QString RootBrowser::id() const { return "rootbrowser"; }

QString RootBrowser::name() const { return "Root browser"; }

QString RootBrowser::defaultTrigger() const { return "/"; }

void RootBrowser::handleQuery(Query &query) const
{
    query.add(buildItems(QString("/%1").arg(query.string())));
}


QString HomeBrowser::id() const { return "homebrowser"; }

QString HomeBrowser::name() const { return "Home browser"; }

QString HomeBrowser::defaultTrigger() const { return "~"; }

void HomeBrowser::handleQuery(Query &query) const
{
    query.add(buildItems(QString("%1%2").arg(QDir::homePath(), query.string())));
}


