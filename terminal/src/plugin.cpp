// Copyright (c) 2017-2024 Manuel Schneider

#include "albert/albert.h"
#include "albert/extension/queryhandler/standarditem.h"
#include "plugin.h"
#include <QDirIterator>
#include <QFileSystemWatcher>
#include <QStringList>
#include <functional>
ALBERT_LOGGING_CATEGORY("terminal")
using namespace albert;
using namespace std;

Plugin::Plugin()
{
    indexer.parallel = [](const bool &abort){
        set<QString> result;
        QStringList paths = QString(::getenv("PATH")).split(':', Qt::SkipEmptyParts);
        DEBG << "Indexing" << paths.join(", ");
        for (const QString &path : paths) {
            QDirIterator dirIt(path, QDir::NoDotAndDotDot|QDir::Files|QDir::Executable, QDirIterator::Subdirectories);
            while (dirIt.hasNext()) {
                if (abort) return result;
                dirIt.next();
                result.insert(dirIt.fileName());
            }
        }
        return result;
    };
    indexer.finish = [this](set<QString> && res){
        INFO << QStringLiteral("Indexed %1 executables [%2 ms]")
                    .arg(res.size()).arg(indexer.runtime.count());
        index = ::move(res);
    };
    
    watcher.addPaths(QString(::getenv("PATH")).split(':', Qt::SkipEmptyParts));
    connect(&watcher, &QFileSystemWatcher::directoryChanged, this, [this](){ indexer.run(); });

    indexer.run();
}

static vector<Action> buildActions(const QString &commandline)
{
    static const auto tr_r = Plugin::tr("Run");
    static const auto tr_rc = Plugin::tr("Run and close on exit");
    static const auto tr_rb = Plugin::tr("Run in background (without terminal)");
    return {
        {
            "r", tr_r,
            [=]() { runTerminal(commandline); }
        },
        {
            "rc", tr_rc,
            [=]() { runTerminal(commandline, {}, true); }
        },
        {
            "rb",
            tr_rb,
            [=]() { runDetachedProcess({"sh", "-c", commandline}); }
        }
    };
}

void Plugin::handleTriggerQuery(TriggerQuery *query) const
{
    if (query->string().trimmed().isEmpty())
        return;

    vector<shared_ptr<Item>> results;

    // Extract data from input string: [0] program. The rest: args
    QString potentialProgram = query->string().section(' ', 0, 0, QString::SectionSkipEmpty);
    QString remainder = query->string().section(' ', 1, -1, QString::SectionSkipEmpty);

    static const auto tr_rcmd = tr("Run '%2'");
    static const QStringList icon_urls{"xdg:utilities-terminal", "xdg:terminal", ":terminal"};

    QString commonPrefix;
    if (auto it = lower_bound(index.begin(), index.end(), potentialProgram); it != index.end()){
        commonPrefix = *it;

        while (it != index.end() && it->startsWith(potentialProgram)) {

            // Update common prefix
            auto mismatchindexes = mismatch(it->begin() + potentialProgram.size() - 1, it->end(),
                                            commonPrefix.begin() + potentialProgram.size() - 1);
            commonPrefix.resize(distance(it->begin(), mismatchindexes.first));

            auto commandline = QString("%1 %2").arg(*it, remainder);

            results.emplace_back(
                StandardItem::make(
                    {},
                    commandline,
                    tr_rcmd.arg(commandline),
                    commonPrefix,
                    icon_urls,
                    buildActions(commandline)
                )
            );
            ++it;
        }

        // Apply completion string to items
        QString completion = QString("%1%2 %3").arg(query->trigger(), commonPrefix, remainder);
        for (auto &item: results)
            static_pointer_cast<StandardItem>(item)->setInputActionText(completion);
    }

    // Build generic item

    static const auto tr_title = tr("I'm Feeling Lucky");
    static const auto tr_description = tr("Try running '%1'");
    results.emplace_back(
        StandardItem::make(
            {},
            tr_title,
            tr_description.arg(query->string()),
            icon_urls,
            buildActions(query->string())
        )
    );

    query->add(results);
}
