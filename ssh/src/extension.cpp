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
}

class Ssh::Private
{
public:
    QString icon;
    QPointer<ConfigWidget> widget;
    QFileSystemWatcher fileSystemWatcher;
    map<QString, QString> hosts;
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

    if (trimmed.isEmpty()) {
        if (query->isTriggered()) {
            for (const auto &pair : d->hosts) {

                const QString &host = pair.first;
                const QString &port = pair.second;

                QString target = port.isEmpty() ? host : QString("[%1]:%2").arg(host, port);

                auto item = std::make_shared<StandardItem>("ssh_" + target);
                item->setText(target);
                item->setSubtext(QString("Connect to '%1'").arg(target));
                item->setCompletion(QString("ssh %1").arg(target));
                item->setIconPath(d->icon);
                item->addAction(make_shared<TermAction>(QString("Connect to '%1'").arg(target), QStringList() << "ssh" << QString("ssh://%1").arg(target)));

                query->addMatch(std::move(item));
            }
        }
    }
    else
    {
        // Check sanity of input
        QRegularExpression re("^(?:(\\w+)@)?([\\w\\–\\.]*)(?::(\\d+))?$");
        QRegularExpressionMatch match = re.match(trimmed);

        if (match.hasMatch())
        {
            QString q_user = match.captured(1);
            QString q_host = match.captured(2);
            QString q_port = match.captured(3);

            // Show all hosts matching the query
            for (const auto &pair : d->hosts) {

                const QString &host = pair.first;
                const QString &port = pair.second;

                if (host.startsWith(q_host, Qt::CaseInsensitive))
                {
                    auto item = std::make_shared<StandardItem>("ssh_"+host);
                    item->setText(host);
                    item->setIconPath(d->icon);

                    QString target = host;
                    if (!q_port.isEmpty())
                        target = QString("[%1]:%2").arg(target, q_port);
                    else if (!port.isEmpty())
                        target = QString("[%1]:%2").arg(target, port);
                    if (!q_user.isEmpty())
                        target = QString("%1@%2").arg(q_user, target);

                    QString subtext = QString("Connect to '%1'").arg(target);

                    item->setSubtext(subtext);
                    item->setCompletion(QString("ssh %1").arg(target));
                    item->addAction(make_shared<TermAction>(subtext, QStringList{"ssh", QString("ssh://%1").arg(target)}));

                    query->addMatch(std::move(item), static_cast<uint>(1.0*q_host.size()/host.size()* UINT_MAX));
                }
            }

            if (query->isTriggered() && !q_host.isEmpty()) {
                // Add the quick connect item
                auto item  = std::make_shared<StandardItem>();
                item->setText(trimmed);
                item->setSubtext("Quick connect to an unknown host");
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

    d->hosts.clear();

    // Get the hosts in config
    for (const QString& path : { QString("/etc/ssh/config"), QDir::home().filePath(".ssh/config") }) {
        if (QFile::exists(path)) {
            QFile file(path);
            if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
                QTextStream in(&file);
                while (!in.atEnd()) {
                    QStringList fields = in.readLine().split(QRegularExpression("\\s+"), QString::SkipEmptyParts);
                    if ( fields.size() > 1 && fields[0] == "Host")
                        for ( int i = 1; i < fields.size(); ++i )
                            if ( !(fields[i].contains('*') || fields[i].contains('?')) )
                                d->hosts.emplace(fields[i], "");
                }
                file.close();
            }
        }
    }

    // Get the hosts in known_hosts
    if (d->useKnownHosts) {
        const QString& path = QDir::home().filePath(".ssh/known_hosts");
        if (QFile::exists(path)){
            QFile file(path);
            if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
                QRegularExpression re("^\\[?([\\w\\-\\.\\:]+)\\]?(?::(\\d+))?");
                QTextStream in(&file);
                while (!in.atEnd()) {
                    QRegularExpressionMatch match = re.match(in.readLine());
                    if (match.hasMatch())
                        d->hosts.emplace(match.captured(1), match.captured(2));
                }
                file.close();
            }
        }
    }
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
