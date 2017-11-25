// albert - a simple application launcher for linux
// Copyright (C) 2014-2017 Manuel Schneider
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#include <QDebug>
#include <QFileInfo>
#include <QFileSystemWatcher>
#include <QDirIterator>
#include <QPointer>
#include <QFuture>
#include <QFutureWatcher>
#include <QProcess>
#include <QStringList>
#include <QtConcurrent>
#include <set>
#include "extension.h"
#include "configwidget.h"
#include "util/shutil.h"
#include "util/standardactions.h"
#include "util/standarditem.h"
#include "xdg/iconlookup.h"
using namespace std;
using namespace Core;

extern QString terminalCommand;

/** ***************************************************************************/
/** ***************************************************************************/
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
        item->addAction(make_shared<ProcAction>("Run", commandline));
        item->addAction(make_shared<TermAction>("Run in terminal", commandline,
                                                QString(), true,
                                                TermAction::CloseBehavior::DoNotClose));

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
    item->addAction(make_shared<ProcAction>("Run", commandline));
    item->addAction(make_shared<TermAction>("Run in terminal", commandline,
                                            QString(), true,
                                            TermAction::CloseBehavior::DoNotClose));

    results.emplace_back(item, 0);

    // Add results to query
    query->addMatches(results.begin(), results.end());
}


/** ***************************************************************************/
void Terminal::Extension::rebuildIndex() {

    if ( d->futureWatcher.isRunning() )
        return;

    auto index = [](){
        qDebug() << "Indexing executables in $PATH.";
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
        qDebug() << "Finished indexing executables in $PATH.";
        return index;
    };

    connect(&d->futureWatcher, &QFutureWatcher<set<QString>>::finished, this, [this](){
        d->index = d->futureWatcher.future().result();
        disconnect(&d->futureWatcher, nullptr, nullptr, nullptr);
    });

    d->futureWatcher.setFuture(QtConcurrent::run(index));
}
