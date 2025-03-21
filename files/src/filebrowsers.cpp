// Copyright (c) 2022 Manuel Schneider

#include "filebrowsers.h"
#include "fileitems.h"
#include <albert/logging.h>
#include <QCoreApplication>
#include <QDir>
#include <QDirIterator>
#include <QFileInfo>
#include <QMimeDatabase>
#include <QFileInfo>
using namespace albert;
using namespace std;

FilePathBrowser::FilePathBrowser(bool &matchCaseSensitive, bool &showHidden,
                                 bool &sortCaseSensitive, bool &showDirsFirst):
    match_case_sensitive_(matchCaseSensitive),
    show_hidden_(showHidden),
    sort_case_insensitive_(sortCaseSensitive),
    show_dirs_first_(showDirsFirst)
{}

bool FilePathBrowser::allowTriggerRemap() const { return false; }

QFileInfoList FilePathBrowser::listFiles(const QString &filter_path) const
{
    QFileInfo query_file_info(filter_path);
    QDir dir(query_file_info.path());

    if (dir.exists())
    {
        auto pattern = query_file_info.fileName() + "*";

        auto filters = QDir::AllEntries | QDir::NoDotAndDotDot;
        if (match_case_sensitive_)
            filters |= QDir::CaseSensitive;
        if (show_hidden_ || query_file_info.fileName().startsWith(QStringLiteral(".")))
            filters |= QDir::Hidden;

        QDir::SortFlags sort_flags = QDir::Name;
        if (sort_case_insensitive_)
            sort_flags |= QDir::IgnoreCase;
        if (show_dirs_first_)
            sort_flags |= QDir::DirsFirst;

        if (pattern.isEmpty())
            return dir.entryInfoList(filters, sort_flags);
        else
            return dir.entryInfoList({pattern}, filters, sort_flags);
    }
    return {};
}

// -------------------------------------------------------------------------------------------------

RootBrowser::RootBrowser(bool &matchCaseSensitive, bool &showHidden,
                         bool &sortCaseSensitive, bool &showDirsFirst):
    FilePathBrowser(matchCaseSensitive, showHidden, sortCaseSensitive, showDirsFirst)
{}

QString RootBrowser::id() const { return "rootbrowser"; }

QString RootBrowser::name() const { return tr("Root browser"); }

QString RootBrowser::description() const { return tr("Browse root directory by path"); }

QString RootBrowser::defaultTrigger() const { return QStringLiteral("/"); }

void RootBrowser::handleTriggerQuery(Query &query)
{
    vector<shared_ptr<Item>> results;
    QMimeDatabase mimeDatabase;
    for (const auto &fi : listFiles(defaultTrigger() + query.string()))
    {
        QMimeType mimetype = mimeDatabase.mimeTypeForFile(fi);
        QString completion = fi.filePath().mid(1);
        if (fi.isDir())
            completion.append(QDir::separator());

        results.emplace_back(make_shared<StandardFile>(fi.filePath(), mimetype, completion));
    }

    query.add(::move(results));
}


// -------------------------------------------------------------------------------------------------

HomeBrowser::HomeBrowser(bool &matchCaseSensitive, bool &showHidden,
                         bool &sortCaseSensitive, bool &showDirsFirst):
    FilePathBrowser(matchCaseSensitive, showHidden, sortCaseSensitive, showDirsFirst)
{}

QString HomeBrowser::id() const { return "homebrowser"; }

QString HomeBrowser::name() const { return tr("Home browser"); }

QString HomeBrowser::description() const { return tr("Browse home directory by path"); }

QString HomeBrowser::defaultTrigger() const { return QStringLiteral("~"); }

void HomeBrowser::handleTriggerQuery(Query &query)
{
    vector<shared_ptr<Item>> results;
    QMimeDatabase mimeDatabase;
    auto home_length = QDir::homePath().size();
    for (const auto &fi : listFiles(QDir::homePath() + query.string()))
    {
        QMimeType mimetype = mimeDatabase.mimeTypeForFile(fi);
        QString completion = fi.filePath().mid(home_length);
        if (fi.isDir())
            completion.append(QDir::separator());

        results.emplace_back(make_shared<StandardFile>(fi.filePath(), mimetype, completion));
    }

    query.add(::move(results));
}


