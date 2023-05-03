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
    QFileInfo query_file_info(input);
    QDir dir(query_file_info.path());
    QString name_filter = query_file_info.fileName();

    if (dir.exists()) {
        QMimeDatabase mimeDatabase;

        for (const QFileInfo &dir_entry_info: dir.entryInfoList(QDir::AllEntries | QDir::Hidden | QDir::NoDotAndDotDot,
                                                                QDir::DirsFirst | QDir::Name | QDir::IgnoreCase)) {
            if (!dir_entry_info.fileName().startsWith(name_filter))
                continue;

            QMimeType mimetype = mimeDatabase.mimeTypeForFile(dir_entry_info);
            QString completion = dir_entry_info.filePath();

            if (dir_entry_info.isDir())
                completion.append(QDir::separator());

            if (completion.startsWith(QDir::homePath()))
                completion = QString("~%1").arg(completion.mid(QDir::homePath().size()));

            results.emplace_back(make_shared<StandardFile>(
                dir_entry_info.filePath(),
                mimetype,
                completion
            ));
        }
    }
    return results;
}


QString RootBrowser::id() const { return "rootbrowser"; }

QString RootBrowser::name() const { return "Root browser"; }

QString RootBrowser::defaultTrigger() const { return "/"; }

void RootBrowser::handleTriggerQuery(TriggerQuery &query) const
{
    query.add(buildItems(QString("/%1").arg(query.string())));
}


QString HomeBrowser::id() const { return "homebrowser"; }

QString HomeBrowser::name() const { return "Home browser"; }

QString HomeBrowser::defaultTrigger() const { return "~"; }

void HomeBrowser::handleTriggerQuery(TriggerQuery &query) const
{
    query.add(buildItems(QString("%1%2").arg(QDir::homePath(), query.string())));
}


