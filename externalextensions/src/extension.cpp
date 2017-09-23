// albert - a simple application launcher for linux
// Copyright (C) 2014-2015 Manuel Schneider
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
#include <QDirIterator>
#include <QFileSystemWatcher>
#include <QPointer>
#include <QProcess>
#include <QStandardPaths>
#include <vector>
#include <memory>
#include "configwidget.h"
#include "extension.h"
#include "externalextension.h"
#include "externalextensionmodel.h"
using namespace std;


class ExternalExtensions::Private
{
public:
    std::vector<std::unique_ptr<ExternalExtension>> externalExtensions;
    QFileSystemWatcher fileSystemWatcher;
    QPointer<ConfigWidget> widget;
};


/** ***************************************************************************/
ExternalExtensions::Extension::Extension()
    : Core::Extension("org.albert.extension.externalextensions"),
      d(new Private) {

    QString oldPath =  QDir(QStandardPaths::writableLocation(QStandardPaths::DataLocation)).filePath("external");
    if (QFile::exists(oldPath))
        QFile::rename(oldPath, dataLocation().filePath("extensions"));

    if ( !dataLocation().exists("extensions") )
        dataLocation().mkdir("extensions");

    connect(&d->fileSystemWatcher, &QFileSystemWatcher::fileChanged,
            this, &Extension::reloadExtensions);

    connect(&d->fileSystemWatcher, &QFileSystemWatcher::directoryChanged,
            this, &Extension::reloadExtensions);

    reloadExtensions();
}



/** ***************************************************************************/
ExternalExtensions::Extension::~Extension() {
    // Unregister
    for ( auto & script : d->externalExtensions )
        if ( script->state() == ExternalExtension::State::Initialized )
            unregisterQueryHandler(script.get());
}



/** ***************************************************************************/
QWidget *ExternalExtensions::Extension::widget(QWidget *parent) {
    if (d->widget.isNull()){
        d->widget = new ConfigWidget(parent);

        ExternalExtensionsModel *model = new ExternalExtensionsModel(d->externalExtensions, d->widget->ui.tableView);
        d->widget->ui.tableView->setModel(model);

        connect(d->widget->ui.tableView, &QTableView::activated,
                model, &ExternalExtensionsModel::onActivated);

//        // Reset the widget when
//        connect(this, &Extension::extensionsUpdated,
//                d->widget->ui.tableView, &QTableView::reset);
    }
    return d->widget;
}



/** ***************************************************************************/
void ExternalExtensions::Extension::reloadExtensions() {

    // Unregister
    for ( auto & script : d->externalExtensions )
        if ( script->state() == ExternalExtension::State::Initialized )
            unregisterQueryHandler(script.get());

    d->externalExtensions.clear();

    // Remove all watches
    if ( !d->fileSystemWatcher.files().isEmpty() )
        d->fileSystemWatcher.removePaths(d->fileSystemWatcher.files());
    if ( !d->fileSystemWatcher.directories().isEmpty() )
        d->fileSystemWatcher.removePaths(d->fileSystemWatcher.directories());

    // Iterate over all files in the plugindirs
    for (const QString &pluginDir : QStandardPaths::locateAll(QStandardPaths::DataLocation,
                                                              Core::Plugin::id(),
                                                              QStandardPaths::LocateDirectory) ) {
        QString extensionDir = QDir(pluginDir).filePath("extensions");
        if ( QFile::exists(extensionDir) ) {

            // Watch this dir
            d->fileSystemWatcher.addPath(extensionDir);

            QDirIterator dirIterator(extensionDir, QDir::Files|QDir::Executable, QDirIterator::NoIteratorFlags);
            while (dirIterator.hasNext()) {

                QString path = dirIterator.next();
                QString id = dirIterator.fileInfo().fileName();

                // Skip if this id already exists
                auto it = find_if(d->externalExtensions.begin(), d->externalExtensions.end(),
                                  [&id](const unique_ptr<ExternalExtension> & rhs){
                    return id == rhs->id();
                });

                if ( it == d->externalExtensions.end() ) {
                    d->externalExtensions.emplace_back(new ExternalExtension(path, QString("org.arlbert.extension.external.%1").arg(id)));
                    d->fileSystemWatcher.addPath(path);
                }
            }
        }
    }

    // Register
    for ( auto & script : d->externalExtensions )
        if ( script->state() == ExternalExtension::State::Initialized )
            registerQueryHandler(script.get());

    emit extensionsUpdated();
}
