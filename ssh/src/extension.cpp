// Copyright (C) 2014-2018 Manuel Schneider

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

    QString trimmed = query->string().trimmed();

    if (trimmed.isEmpty())
    {
        if (query->isTriggered())
        {
            // Show all hosts
            for (const QString& host : d->hosts) {
                auto item = std::make_shared<StandardItem>("ssh_"+host);
                item->setText(host);
                item->setSubtext(QString("Connect to '%1' using ssh").arg(host));
                item->setCompletion(QString("ssh %1").arg(host));
                item->setIconPath(d->icon);
                item->addAction(make_shared<TermAction>(QString("Connect to '%1'").arg(host), QStringList() << "ssh" << host));
                query->addMatch(std::move(item), static_cast<uint>(1.0*query->string().size()/host.size()* UINT_MAX));
            }
        }
    }
    else
    {
        // Check sanity of input
        QRegularExpression re("^(\\w+@)?([\\w.]*)$");
        QRegularExpressionMatch match = re.match(trimmed);

        if (match.hasMatch())
        {
            QString q_user = match.captured(1);
            QString q_host = match.captured(2);

            // Show all hosts matching the query
            for (const QString& host : d->hosts)
            {
                if (host.startsWith(q_host, Qt::CaseInsensitive))
                {
                    auto item = std::make_shared<StandardItem>("ssh_"+host);
                    item->setText(host);
                    item->setIconPath(d->icon);
                    if (q_user.isEmpty())
                    {
                        item->setSubtext(QString("Connect to '%1'").arg(host));
                        item->setCompletion(QString("ssh %1").arg(host));
                        item->addAction(make_shared<TermAction>(QString("Connect to '%1'").arg(host), QStringList() << "ssh" << host));
                    }
                    else
                    {
                        item->setSubtext(QString("Connect to '%1' as '%2'").arg(host, q_user.chopped(1)));
                        item->setCompletion(QString("ssh %1%2").arg(q_user, host));
                        item->addAction(make_shared<TermAction>(QString("Connect to '%1' as '%2'").arg(host, q_user.chopped(1)), QStringList() << "ssh" << QString("%1%2").arg(q_user, host)));
                    }
                    query->addMatch(std::move(item), static_cast<uint>(1.0*q_host.size()/host.size()* UINT_MAX));
                }
            }

            if (query->isTriggered()) {
                // Add the quick connect item
                auto item  = std::make_shared<StandardItem>();
                item->setText(trimmed);
                item->setSubtext("Quick connect to a host not listed in config");
                item->setCompletion(QString("ssh %1").arg(trimmed));
                item->setIconPath(d->icon);
                item->addAction(make_shared<TermAction>(QString("Connect to '%1'").arg(match.captured(0)), QStringList() << "ssh" << trimmed));
                query->addMatch(std::move(item));
            }
        }
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
