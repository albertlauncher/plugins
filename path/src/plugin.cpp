// Copyright (c) 2017-2024 Manuel Schneider

#include "plugin.h"
#include <QDirIterator>
#include <QFileSystemWatcher>
#include <QLabel>
#include <QStringList>
#include <albert/albert.h>
#include <albert/extensionregistry.h>
#include <albert/standarditem.h>
#include <functional>
ALBERT_LOGGING_CATEGORY("terminal")
using namespace albert;
using namespace std;

Plugin::Plugin()
{
    indexer_.parallel = [](const bool &abort){
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

    indexer_.finish = [this](set<QString> && res){
        INFO << QStringLiteral("Indexed %1 executables [%2 ms]")
                    .arg(res.size()).arg(indexer_.runtime.count());
        index_ = ::move(res);
    };

    watcher_.addPaths(QString(::getenv("PATH")).split(':', Qt::SkipEmptyParts));
    connect(&watcher_, &QFileSystemWatcher::directoryChanged, this, [this](){ indexer_.run(); });

    indexer_.run();
}

Plugin::~Plugin() = default;

QWidget *Plugin::buildConfigWidget()
{
    QString t = QString(R"(<ul style="margin-left:-1em">)");
    for (const auto & path : QString(::getenv("PATH")).split(':', Qt::SkipEmptyParts))
        t += QString(R"(<li><a href="file://%1")>%1</a></li>)").arg(path);
    t +=  "</ul>";

    auto l = new QLabel(t);
    l->setOpenExternalLinks(true);
    l->setWordWrap(true);
    l->setAlignment(Qt::AlignTop);
    return l;
}

QString Plugin::synopsis(const QString &) const { return tr("<command> [params]"); }

QString Plugin::defaultTrigger() const { return ">"; }

void Plugin::setTrigger(const QString &trigger) { trigger_ = trigger; }

vector<Action> Plugin::buildActions(const QString &commandline) const
{
    vector<Action> a;

    a.emplace_back("r", tr("Run in terminal"),
                   [=, this]{ apps_->runTerminal(QString("%1 ; exec $SHELL").arg(commandline)); });

    a.emplace_back("rc", tr("Run in terminal and close on exit"),
                   [=, this]{ apps_->runTerminal(commandline); });

    a.emplace_back("rb", tr("Run in background (without terminal)"),
                   [=]{ runDetachedProcess({"sh", "-c", commandline}); });

    return a;
}

void Plugin::handleTriggerQuery(Query &query)
{
    if (query.string().trimmed().isEmpty())
        return;

    vector<shared_ptr<Item>> results;

    // Extract data from input string: [0] program. The rest: args
    QString potentialProgram = query.string().section(' ', 0, 0, QString::SectionSkipEmpty);
    QString remainder = query.string().section(' ', 1, -1, QString::SectionSkipEmpty);

    static const auto tr_rcmd = tr("Run '%1'");
    static const QStringList icon_urls{"xdg:utilities-terminal", "xdg:terminal", ":path"};

    QString commonPrefix;
    if (auto it = lower_bound(index_.begin(), index_.end(), potentialProgram); it != index_.end()){
        commonPrefix = *it;

        while (it != index_.end() && it->startsWith(potentialProgram)) {

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
        QString completion = QString("%1%2 %3").arg(trigger_, commonPrefix, remainder);
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
            tr_description.arg(query.string()),
            icon_urls,
            buildActions(query.string())
        )
    );

    query.add(results);
}
