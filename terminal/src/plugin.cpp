// Copyright (c) 2022-2023 Manuel Schneider

#include "albert.h"
#include "albert/util/util.h"
#include "plugin.h"
#include <QDirIterator>
#include <QFileSystemWatcher>
#include <QStringList>
#include <QtConcurrent>
#include <functional>
ALBERT_LOGGING
using namespace std;
using namespace albert;

static QStringList icon_urls{"xdg:utilities-terminal", "xdg:terminal", ":terminal"};

::Plugin::Plugin()
{
    indexer.parallel = [](const bool &abort){
        INFO << "Indexing $PATH.";
        set<QString> result;
        QStringList paths = QString(::getenv("PATH")).split(':', Qt::SkipEmptyParts);
        for (const QString &path : paths) {
            QDirIterator dirIt(path, QDir::NoDotAndDotDot|QDir::Files|QDir::Executable, QDirIterator::Subdirectories);
            while (dirIt.hasNext()) {
                if (abort) return result;
                dirIt.next();
                result.insert(dirIt.fileName());
            }
        }
        DEBG << "Finished indexing $PATH.";
        return result;
    };
    indexer.finish = [this](set<QString> && res){
        index = ::move(res);
    };
    
    
    watcher.addPaths(QString(::getenv("PATH")).split(':', Qt::SkipEmptyParts));
    connect(&watcher, &QFileSystemWatcher::directoryChanged, this, [this](){ indexer.run(); });

    indexer.run();
}

void ::Plugin::handleTriggerQuery(TriggerQuery &query) const
{
    if (query.string().trimmed().isEmpty())
        return;

    vector<shared_ptr<Item>> results;

    // Extract data from input string: [0] program. The rest: args
    QString potentialProgram = query.string().section(' ', 0, 0, QString::SectionSkipEmpty);
    QString remainder = query.string().section(' ', 1, -1, QString::SectionSkipEmpty);

    QString commonPrefix;
    if (auto it = lower_bound(index.begin(), index.end(), potentialProgram); it != index.end()){
        commonPrefix = *it;

        while (it != index.end() && it->startsWith(potentialProgram)) {

            // Update common prefix
            auto mismatchindexes = std::mismatch(it->begin() + potentialProgram.size() - 1, it->end(),
                                                 commonPrefix.begin() + potentialProgram.size() - 1);
            commonPrefix.resize(std::distance(it->begin(), mismatchindexes.first));

            auto commandline = QString("%1 %2").arg(*it, remainder);

            results.emplace_back(StandardItem::make(
                    {},
                    commandline,
                    QString("Run '%1'").arg(commandline),
                    commonPrefix,
                    icon_urls,
                    {{"", "Run",
                      [=]() { runTerminal(commandline); }},
                     {"", "Run and close on exit",
                      [=]() { runTerminal(commandline, {}, true); }},
                     {"", "Run in background (without terminal)",
                      [=]() { runDetachedProcess({"sh", "-c", commandline}); }}}
            ));
            ++it;
        }

        // Apply completion string to items
        QString completion = QString("%1%2 %3").arg(query.trigger(), commonPrefix, remainder);
        for (auto &item: results)
            std::static_pointer_cast<StandardItem>(item)->setInputActionText(completion);
    }

    // Build generic item
    results.emplace_back(StandardItem::make(
            {},
            "I'm Feeling Lucky",
            QString("Try running '%1'").arg(query.string()),
            icon_urls,
            {{"", "Run",
              [cmdln=query.string()](){ runTerminal(cmdln); }},
             {"", "Run and close on exit",
              [cmdln=query.string()](){ runTerminal(cmdln, {}, true); }},
             {"", "Run in background (without terminal)",
              [cmdln=query.string()](){ runDetachedProcess({"sh", "-c", cmdln}); }}}
    ));

    query.add(results);
}
