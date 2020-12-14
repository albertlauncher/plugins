// Copyright (C) 2014-2018 Manuel Schneider

#include <QDirIterator>
#include <QFileInfo>
#include <QFileSystemWatcher>
#include <QFuture>
#include <QFutureWatcher>
#include <QPointer>
#include <QProcess>
#include <QStringList>
#include <QtConcurrent>
#include <QWidget>
#include <QLabel>
#include <QVBoxLayout>
#include <QSpacerItem>
#include <set>
#include <functional>
#include "albert/util/standardactions.h"
#include "albert/util/standarditem.h"
#include "extension.h"
#include "xdg/iconlookup.h"
Q_LOGGING_CATEGORY(qlc, "term")
#define DEBG qCDebug(qlc,).noquote()
#define INFO qCInfo(qlc,).noquote()
#define WARN qCWarning(qlc,).noquote()
#define CRIT qCCritical(qlc,).noquote()
using namespace std;
using namespace Core;


/** ***************************************************************************/
/** ***************************************************************************/
class Terminal::Extension::Private
{
public:
    QPointer<QWidget> widget;
    QString iconPath;
    QFileSystemWatcher watcher;
    set<QString> index;
    QFutureWatcher<set<QString>> futureWatcher;


    void rebuildIndex() {

        if ( futureWatcher.isRunning() )
            return;

        QObject::connect(&futureWatcher, &QFutureWatcher<set<QString>>::finished, [this](){
            index = futureWatcher.future().result();
            QObject::disconnect(&futureWatcher, nullptr, nullptr, nullptr);
        });

        futureWatcher.setFuture(QtConcurrent::run(indexPath));
    }


    static set<QString> indexPath(){
        INFO << "Indexing $PATH.";
        set<QString> index;
        QStringList paths = QString(::getenv("PATH")).split(':', QString::SkipEmptyParts);
        for (const QString &path : paths) {
            QDirIterator dirIt(path, QDir::NoDotAndDotDot|QDir::Files|QDir::Executable, QDirIterator::Subdirectories);
            while (dirIt.hasNext()) {
                dirIt.next();
                index.insert(dirIt.fileName());
            }
        }
        DEBG << "Finished indexing $PATH.";
        return index;
    };

};


/** ***************************************************************************/
/** ***************************************************************************/
Terminal::Extension::Extension() : Core::Extension("org.albert.extension.terminal"),
                                    Core::QueryHandler(Core::Plugin::id()),
      d(new Private) {

    registerQueryHandler(this);

    if ((d->iconPath = XDG::IconLookup::iconPath("utilities-terminal", "terminal")).isNull())
        d->iconPath = ":terminal";

    d->watcher.addPaths(QString(::getenv("PATH")).split(':', QString::SkipEmptyParts));
    connect(&d->watcher, &QFileSystemWatcher::directoryChanged,
            this, [this](){ d->rebuildIndex(); });

    d->rebuildIndex();
}

Terminal::Extension::~Extension() { }


/** ***************************************************************************/
QWidget *Terminal::Extension::widget(QWidget *parent) {
    if (d->widget.isNull()){
        d->widget = new QWidget(parent);
        QVBoxLayout *verticalLayout = new QVBoxLayout();
        d->widget->setLayout(verticalLayout);

        QLabel *label = new QLabel();
        label->setWordWrap(true);
        label->setText(QCoreApplication::translate("Terminal::ConfigWidget",
            "The terminal extension allows you to run commands in a terminal or "
            "a shell directly. Theres not much more about it but convenience. "
            "Just invoke the extension using the trigger '>'.", nullptr));
        verticalLayout->addWidget(label);

        QSpacerItem *verticalSpacer = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);
        verticalLayout->addItem(verticalSpacer);

    }
    return d->widget;
}



/** ***************************************************************************/
void Terminal::Extension::handleQuery(Core::Query * query) const {

    if ( !query->isTriggered() || query->string().trimmed().isEmpty() )
        return;

    vector<pair<shared_ptr<Core::Item>,short>> results;

    // Extract data from input string: [0] program. The rest: args
    QString potentialProgram = query->string().section(' ', 0, 0, QString::SectionSkipEmpty);
    QString remainder = query->string().section(' ', 1, -1, QString::SectionSkipEmpty);

    // Iterate over matches
    set<QString>::iterator it = lower_bound(d->index.begin(), d->index.end(), potentialProgram);

    // Get the
    QString commonPrefix;
    if ( it != d->index.end() )
        commonPrefix = *it;

    while (it != d->index.end() && it->startsWith(potentialProgram)){

        // Update common prefix
        auto mismatchindexes = std::mismatch(it->begin()+potentialProgram.size()-1, it->end(),
                                             commonPrefix.begin()+potentialProgram.size()-1);
        commonPrefix.resize(std::distance(it->begin(), mismatchindexes.first));

        auto commandline = QString("%1 %2").arg(*it, remainder);

        auto item = makeStdItem(QString(),
            d->iconPath, commandline, QString("Run '%1'").arg(commandline),
            ActionList {
                makeTermAction("Run", commandline, TermAction::DoNotClose),
                makeTermAction("Run and close on exit", commandline, TermAction::CloseOnExit),
                makeTermAction("Run and close on success", commandline, TermAction::CloseOnSuccess),
                makeProcAction("Run in background (without terminal)", QStringList() << "sh" << "-c" << commandline)
            },
            commonPrefix
        );
        results.emplace_back(item, 0);
        ++it;
    }

    // Apply completion string to items
    QString completion = QString(">%1").arg(commonPrefix);
    for (pair<shared_ptr<Core::Item>,short> &match: results)
        std::static_pointer_cast<StandardItem>(match.first)->setCompletion(completion);

    // Build generic item
    auto item = makeStdItem(QString(),
        d->iconPath, "I'm Feeling Lucky", QString("Try running '%1'").arg(query->string()),
        ActionList {
            makeTermAction("Run", query->string(), TermAction::DoNotClose),
            makeTermAction("Run and close on exit", query->string(), TermAction::CloseOnExit),
            makeTermAction("Run and close on success", query->string(), TermAction::CloseOnSuccess),
            makeProcAction("Run in background (without terminal)", QStringList() << "sh" << "-c" << query->string())
        }
    );
    results.emplace_back(move(item), 0);

    // Add results to query
    query->addMatches(results.begin(), results.end());
}
