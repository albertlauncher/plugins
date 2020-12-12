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
#include <set>
#include "albert/util/shutil.h"
#include "albert/util/standardactions.h"
#include "albert/util/standarditem.h"
#include "configwidget.h"
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
class Terminal::Private
{
public:
    QPointer<ConfigWidget> widget;
    QString iconPath;
    QFileSystemWatcher watcher;
    set<QString> index;
    QFutureWatcher<set<QString>> futureWatcher;
};



/** ***************************************************************************/
Terminal::Extension::Extension()
    : Core::Extension("org.albert.extension.terminal"),
      Core::QueryHandler(Core::Plugin::id()),
      d(new Private) {

    registerQueryHandler(this);

    QString iconPath = XDG::IconLookup::iconPath("utilities-terminal", "terminal");
    d->iconPath = iconPath.isNull() ? ":terminal" : iconPath;

    d->watcher.addPaths(QString(::getenv("PATH")).split(':', QString::SkipEmptyParts));
    connect(&d->watcher, &QFileSystemWatcher::directoryChanged,
            this, &Extension::rebuildIndex);

    rebuildIndex();
}



/** ***************************************************************************/
Terminal::Extension::~Extension() {

}



/** ***************************************************************************/
QWidget *Terminal::Extension::widget(QWidget *parent) {
    if (d->widget.isNull())
        d->widget = new ConfigWidget(parent);
    return d->widget;
}



/** ***************************************************************************/
void Terminal::Extension::handleQuery(Core::Query * query) const {

    if ( !query->isTriggered() || query->string().trimmed().isEmpty() )
        return;

    vector<pair<shared_ptr<Core::Item>,short>> results;

    // Extract data from input string: [0] program. The rest: args
    QString potentialProgram = query->string().section(' ', 0, 0, QString::SectionSkipEmpty);
    QStringList arguments = ShUtil::split(query->string().section(' ', 1, -1, QString::SectionSkipEmpty));

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

        QStringList commandline(*it);
        commandline << arguments;

        auto item = make_shared<StandardItem>(*it);
        item->setIconPath(d->iconPath);
        item->setText(commandline.join(' '));
        item->setSubtext(QString("Run '%1'").arg(item->text()));
        item->setCompletion(item->text());
        item->addAction(make_shared<TermAction>("Run", commandline, QString(),
                                                true, TermAction::CloseBehavior::DoNotClose));
        item->addAction(make_shared<TermAction>("Run and close on exit", commandline, QString(),
                                                true, TermAction::CloseBehavior::CloseOnExit));
        item->addAction(make_shared<TermAction>("Run and close on success", commandline, QString(),
                                                true, TermAction::CloseBehavior::CloseOnSuccess));
        item->addAction(make_shared<ProcAction>("Run in background (without terminal)", commandline));
        results.emplace_back(item, 0);
        ++it;
    }

    // Apply completion string to items
    QString completion = QString(">%1").arg(commonPrefix);
    for (pair<shared_ptr<Core::Item>,short> &match: results)
        std::static_pointer_cast<StandardItem>(match.first)->setCompletion(completion);

    // Build generic item
    QStringList commandline = ShUtil::split(query->string());

    auto item = make_shared<StandardItem>();
    item->setIconPath(d->iconPath);
    item->setText("I'm Feeling Lucky");
    item->setSubtext(QString("Try running '%1'").arg(query->string()));
    item->setCompletion(query->rawString());
    item->addAction(make_shared<TermAction>("Run", commandline, QString(),
                                            true, TermAction::CloseBehavior::DoNotClose));
    item->addAction(make_shared<TermAction>("Run and close on exit", commandline, QString(),
                                            true, TermAction::CloseBehavior::CloseOnExit));
    item->addAction(make_shared<TermAction>("Run and close on success", commandline, QString(),
                                            true, TermAction::CloseBehavior::CloseOnSuccess));
    item->addAction(make_shared<ProcAction>("Run in background (without terminal)", commandline));
    results.emplace_back(item, 0);

    // Add results to query
    query->addMatches(results.begin(), results.end());
}


/** ***************************************************************************/
void Terminal::Extension::rebuildIndex() {

    if ( d->futureWatcher.isRunning() )
        return;

    auto index = [](){
        DEBG << "Indexing executables in $PATH.";
        set<QString> index;
        QStringList paths = QString(::getenv("PATH")).split(':', QString::SkipEmptyParts);
        for (const QString &path : paths) {
            QDirIterator dirIt(path);
            while (dirIt.hasNext()) {
                QFileInfo file(dirIt.next());
                if ( file.isExecutable() )
                    index.insert(file.fileName());
            }
        }
        DEBG << "Finished indexing executables in $PATH.";
        return index;
    };

    connect(&d->futureWatcher, &QFutureWatcher<set<QString>>::finished, this, [this](){
        d->index = d->futureWatcher.future().result();
        disconnect(&d->futureWatcher, nullptr, nullptr, nullptr);
    });

    d->futureWatcher.setFuture(QtConcurrent::run(index));
}
