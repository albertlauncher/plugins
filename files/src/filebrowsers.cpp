// Copyright (c) 2022 Manuel Schneider

#include "filebrowsers.h"
#include "fileitems.h"
#include <albert/logging.h>
#include <QCoreApplication>
#include <QDir>
#include <QDirIterator>
#include <QFileInfo>
#include <QMimeDatabase>
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

void FilePathBrowser::handle_(Query &query, const QString &query_string) const
{
    vector<shared_ptr<Item>> results;
    QFileInfo query_file_info(query_string);
    QDir dir(query_file_info.path());

    if (dir.exists())
    {
        auto pattern = query_file_info.fileName() + "*";

        auto filters = QDir::AllEntries | QDir::Hidden | QDir::NoDotAndDotDot;
        if (match_case_sensitive_)
            filters |= QDir::CaseSensitive;
        if (show_hidden_)
            filters |= QDir::Hidden;

        QDir::SortFlags sort_flags = QDir::Name;
        if (sort_case_insensitive_)
            sort_flags |= QDir::IgnoreCase;
        if (show_dirs_first_)
            sort_flags |= QDir::DirsFirst;

        QFileInfoList entry_info_list;
        if (pattern.isEmpty())
            entry_info_list = dir.entryInfoList(filters, sort_flags);
        else
            entry_info_list = dir.entryInfoList({pattern}, filters, sort_flags);


        QMimeDatabase mimeDatabase;
        for (const auto &fi : entry_info_list)
        {
            QMimeType mimetype = mimeDatabase.mimeTypeForFile(fi);
            QString completion = fi.filePath();

            if (fi.isDir())
                completion.append(QDir::separator());

            if (completion.startsWith(QDir::homePath()))
                completion = QString("~%1").arg(completion.mid(QDir::homePath().size()));

            results.emplace_back(make_shared<StandardFile>(
                fi.filePath(),
                mimetype,
                completion
            ));
        }

        query.add(::move(results));
    }
}


// -------------------------------------------------------------------------------------------------

RootBrowser::RootBrowser(bool &matchCaseSensitive, bool &showHidden,
                         bool &sortCaseSensitive, bool &showDirsFirst):
    FilePathBrowser(matchCaseSensitive, showHidden, sortCaseSensitive, showDirsFirst)
{}

QString RootBrowser::id() const { return "rootbrowser"; }

QString RootBrowser::name() const { return tr("Root browser"); }

QString RootBrowser::description() const { return tr("Browse root directory by path"); }

QString RootBrowser::defaultTrigger() const { return "/"; }

void RootBrowser::handleTriggerQuery(Query *query)
{ return handle_(*query, QString("/%1").arg(query->string())); }


// -------------------------------------------------------------------------------------------------

HomeBrowser::HomeBrowser(bool &matchCaseSensitive, bool &showHidden,
                         bool &sortCaseSensitive, bool &showDirsFirst):
    FilePathBrowser(matchCaseSensitive, showHidden, sortCaseSensitive, showDirsFirst)
{}

QString HomeBrowser::id() const { return "homebrowser"; }

QString HomeBrowser::name() const { return tr("Home browser"); }

QString HomeBrowser::description() const { return tr("Browse home directory by path"); }

QString HomeBrowser::defaultTrigger() const { return "~"; }

void HomeBrowser::handleTriggerQuery(Query *query)
{ return handle_(*query, QString("%1%2").arg(QDir::homePath(), query->string())); }


