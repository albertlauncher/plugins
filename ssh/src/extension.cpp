// Copyright (C) 2014-2017 Manuel Schneider

#include <QDebug>
#include <QFileSystemWatcher>
#include <QFile>
#include <QDir>
#include <QPointer>
#include <QProcess>
#include <QRegularExpression>
#include <QSettings>
#include <QStandardPaths>
#include <QString>
#include <memory>
#include <stdexcept>
#include <set>
#include "extension.h"
#include "configwidget.h"
#include "util/standardactions.h"
#include "util/standarditem.h"
#include "util/shutil.h"
#include "xdg/iconlookup.h"
using namespace std;
using namespace Core;

extern QString terminalCommand;

namespace {

const char* CFG_USE_KNOWN_HOSTS = "use_known_hosts";
const bool  DEF_USE_KNOWN_HOSTS = true;

// Function to extract the hosts of a ssh config file
std::set<QString> getSshHostsFromConfig(const QString& path) {
    std::set<QString> hosts;
    QFile file(path);
        if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&file);
        while ( !in.atEnd() ) {
            QStringList fields = in.readLine().split(QRegularExpression("\\s+"), QString::SkipEmptyParts);
            if ( fields.size() > 1 && fields[0] == "Host")
                for ( int i = 1; i < fields.size(); ++i ){
                    if ( !(fields[i].contains('*') || fields[i].contains('?')) )
                        hosts.insert(fields[i]);
                }
        }
        file.close();
    }
    return hosts;
}

// Function to extract the hosts of a ssh known_hosts file
std::set<QString> getSshHostsFromKnownHosts(const QString& path) {
    std::set<QString> hosts;
    QFile file(path);
    if ( file.open(QIODevice::ReadOnly | QIODevice::Text) ) {
        QRegularExpression re ("^[a-zA-Z0-9\\-\\.]*(?=(,.*)*\\s)");
        QTextStream in(&file);
        while ( !in.atEnd() ) {
            QString line = in.readLine();
            QRegularExpressionMatch match = re.match(line);
            if ( match.hasMatch() )
                hosts.insert(match.captured(0));
        }
        file.close();
    }
    return hosts;
}

}



class Ssh::Private
{
public:
    QString icon;
    QPointer<ConfigWidget> widget;
    QFileSystemWatcher fileSystemWatcher;
    set<QString> hosts;

    bool useKnownHosts;
};


/** ***************************************************************************/
/** ***************************************************************************/
/** ***************************************************************************/
/** ***************************************************************************/
Ssh::Extension::Extension()
    : Core::Extension("org.albert.extension.ssh"),
      QueryHandler(Plugin::id()),
      d(new Private) {

    registerQueryHandler(this);

    // Load settings
    d->useKnownHosts = settings().value(CFG_USE_KNOWN_HOSTS, DEF_USE_KNOWN_HOSTS).toBool();

    // Find ssh
    if (QStandardPaths::findExecutable("ssh").isNull())
        throw QString("[%s] ssh not found.").arg(Plugin::id());

    // Find an appropriate icon
    d->icon = XDG::IconLookup::iconPath({"ssh", "terminal"});
    if (d->icon.isEmpty())
        d->icon = ":ssh"; // Fallback

    rescan();
}



/** ***************************************************************************/
Ssh::Extension::~Extension() {

}



/** ***************************************************************************/
QWidget *Ssh::Extension::widget(QWidget *parent) {
    if (d->widget.isNull()) {
        d->widget = new ConfigWidget(parent);

        // Checkboxes
        d->widget->ui.checkBox_knownhosts->setChecked(useKnownHosts());
        connect(d->widget->ui.checkBox_knownhosts, &QCheckBox::toggled,
                this, &Extension::setUseKnownHosts);

        connect(d->widget->ui.pushButton_rescan, &QPushButton::clicked,
                this, &Extension::rescan);

    }
    return d->widget;
}



/** ***************************************************************************/
void Ssh::Extension::handleQuery(Query * query) const {

    if ( !query->string().trimmed().isEmpty() || query->isTriggered() ) {
        // Add all hosts that the query is a prefix of
        QRegularExpression re(QString("(%1)").arg(query->string()), QRegularExpression::CaseInsensitiveOption);
        for ( const QString& host : d->hosts ) {
            if ( host.startsWith(query->string()) ) {

                // Create item
                auto item = std::make_shared<StandardItem>("ssh_"+host);
                item->setText(QString(host).replace(re, "<u>\\1</u>"));
                item->setSubtext(QString("Connect to '%1' using ssh").arg(host));
                item->setCompletion(QString("ssh %1").arg(host));
                item->setIconPath(d->icon);
                item->addAction(make_shared<TermAction>(QString("Connect to '%1' using ssh").arg(host),
                                                        QStringList() << "ssh" << host));
                query->addMatch(item, static_cast<uint>(1.0*query->string().size()/host.size()* UINT_MAX));
            }
        }
    }

    if ( query->isTriggered() ) {

        // Add the quick connect item
        QString trimmed = query->string().trimmed();

        auto item  = std::make_shared<StandardItem>();
        item->setText(trimmed);
        item->setSubtext(QString("Quick connect to '%1' using ssh").arg(trimmed));
        item->setCompletion(QString("ssh %1").arg(trimmed));
        item->setIconPath(d->icon);
        item->addAction(make_shared<TermAction>(QString("Connect to '%1' using ssh").arg(trimmed),
                                                QStringList() << "ssh" << trimmed));
        query->addMatch(std::move(item));
    }
}



/** ***************************************************************************/
void Ssh::Extension::rescan() {

    set<QString> hosts;

    // Get the hosts in config
    for ( const QString& path : {QString("/etc/ssh/config"), QDir::home().filePath(".ssh/config")} )
        if ( QFile::exists(path) )
            for ( const QString& host : getSshHostsFromConfig(path) )
                hosts.insert(host);

    // Get the hosts in known_hosts
    if ( d->useKnownHosts ) {
        const QString& path = QDir::home().filePath(".ssh/known_hosts");
        if ( QFile::exists(path) )
            for ( const QString& host : getSshHostsFromKnownHosts(path) )
                hosts.insert(host);
    }

    d->hosts = std::move(hosts);
}



/** ***************************************************************************/
bool Ssh::Extension::useKnownHosts() {
    return d->useKnownHosts;
}



/** ***************************************************************************/
void Ssh::Extension::setUseKnownHosts(bool b) {
    settings().setValue(CFG_USE_KNOWN_HOSTS, b);
    d->useKnownHosts = b;
    rescan();
}
