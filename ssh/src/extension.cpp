// Copyright (C) 2014-2020 Manuel Schneider

#include <QDir>
#include <QFile>
#include <QFileSystemWatcher>
#include <QPointer>
#include <QLoggingCategory>
#include <QProcess>
#include <QRegularExpression>
#include <QSettings>
#include <QStandardPaths>
#include <QString>
#include <QTextStream>
#include <memory>
#include <set>
#include <stdexcept>
#include "albert/util/standardactions.h"
#include "albert/util/standarditem.h"
#include "configwidget.h"
#include "extension.h"
#include "xdg/iconlookup.h"
Q_LOGGING_CATEGORY(qlc, "ssh")
#define DEBG qCDebug(qlc,).noquote()
#define INFO qCInfo(qlc,).noquote()
#define WARN qCWarning(qlc,).noquote()
#define CRIT qCCritical(qlc,).noquote()
using namespace std;
using namespace Core;

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
    vector<pair<QString, QString>> hosts;
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
    if ((d->icon = XDG::IconLookup::iconPath({"ssh", "terminal"})).isNull())
        d->icon = ":ssh";

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

                query->addMatch(makeStdItem("ssh_" + target,
                    d->icon, target, QString("Connect to '%1'").arg(target),
                    ActionList{
                        makeTermAction(QString("Connect to '%1'").arg(target),QStringList{"ssh", target})
                    }
                ));
            }
        }
    }
    else
    {
        // Check sanity of input
        QRegularExpression re(R"raw(^(?:(\w+)@)?\[?((?:\w[\w:-]*|[\w\.-]*))\]?(?::(\d+))?$)raw");
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

                    QString target = host;
                    if (!q_port.isEmpty())
                        target = QString("[%1]:%2").arg(target, q_port);
                    else if (!port.isEmpty())
                        target = QString("[%1]:%2").arg(target, port);
                    if (!q_user.isEmpty())
                        target = QString("%1@%2").arg(q_user, target);
                    QString subtext = QString("Connect to '%1'").arg(target);

                    auto item = makeStdItem("ssh_"+host,
                                            d->icon,
                                            host,
                                            subtext,
                                            ActionList { makeTermAction(subtext, QStringList{"ssh", target}) },
                                            QString("ssh %1").arg(target));

                    query->addMatch(std::move(item), static_cast<uint>(1.0*q_host.size()/host.size()* UINT_MAX));
                }
            }

            if (query->isTriggered() && !q_host.isEmpty()) {
                // Add the quick connect item
                query->addMatch(makeStdItem(
                    "",
                    d->icon,
                    trimmed,
                    "Quick connect to an unknown host",
                    ActionList {
                        makeTermAction(QString("Connect to '%1'").arg(match.captured(0)),
                                       QStringList{"ssh", trimmed})
                    },
                    QString("ssh %1").arg(trimmed)
                ));
            }
        }
    }
}



/** ***************************************************************************/
void Ssh::Extension::rescan() {

    map<QString, QString> hosts;

    std::function<void(const QString&)> scanfile = [&scanfile, &hosts](const QString& path) {
        if (QFile::exists(path)) {
            QFile file(path);
            if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
                QTextStream in(&file);
                while (!in.atEnd()) {
                    QStringList fields = in.readLine().split(QRegularExpression("\\s+"), QString::SkipEmptyParts);
                    if ( fields.size() > 1 && fields[0] == "Host") {
                        for ( int i = 1; i < fields.size(); ++i )
                            if ( !(fields[i].contains('*') || fields[i].contains('?')) )
                                hosts.emplace(fields[i], QString());
                    } else if (fields.size() > 1 && fields[0] == "Include") {
                        // TODO move this somewhere else
                        // TODO make it proper tilde expansion
                        if (fields[1][0] == '~') {
                            fields[1].remove(0,1).insert(0, QDir::homePath());
                        }
                        scanfile(fields[1]);
                    }
                }
                file.close();
            }
        }
    };

    // Get the hosts in config
    for (const QString& path : { QString("/etc/ssh/config"), QDir::home().filePath(".ssh/config") })
        scanfile(path);

    // Get the hosts in known_hosts
    if (d->useKnownHosts) {
        const QString& path = QDir::home().filePath(".ssh/known_hosts");
        if (QFile::exists(path)){
            QFile file(path);
            if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
                QRegularExpression re(R"raw(^\[?([\w\-\.\:]+)\]?(?::(\d+))?)raw");
                QTextStream in(&file);
                while (!in.atEnd()) {
                    QRegularExpressionMatch match = re.match(in.readLine());
                    if (match.hasMatch())
                        hosts.emplace(match.captured(1), match.captured(2));
                }
                file.close();
            }
        }
    }

    d->hosts = vector<pair<QString, QString>>{hosts.begin(), hosts.end()};

    // Sort by length and lexical
    std::sort(d->hosts.begin(), d->hosts.end(),
              [](const auto &li, const auto &ri){ return li.first < ri.first; });
    std::stable_sort(d->hosts.begin(), d->hosts.end(),
                    [](const auto &li, const auto &ri){ return li.first.size() < ri.first.size(); });
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
