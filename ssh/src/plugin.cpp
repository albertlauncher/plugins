// Copyright (c) 2017-2024 Manuel Schneider

#include "plugin.h"
#include <QDir>
#include <QFile>
#include <QLabel>
#include <QRegularExpression>
#include <QString>
#include <QTextStream>
#include <QWidget>
#include <albert/logging.h>
#include <albert/standarditem.h>
#include <albert/util.h>
ALBERT_LOGGING_CATEGORY("ssh")
using namespace albert;
using namespace std;

const QStringList Plugin::icon_urls = {"xdg:ssh", ":ssh"};

const QRegularExpression Plugin::regex_synopsis = QRegularExpression(R"raw(^(?:(\w+)@)?\[?([\w\.-]*)\]?(?:\h+(.*))?$)raw");

static QSet<QString> parseConfigFile(const QString &path)
{
    QSet<QString> hosts;

    if (QFile file(path); file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QTextStream in(&file);
        while (!in.atEnd())
        {
            QStringList fields = in.readLine().split(" ", Qt::SkipEmptyParts);
            if (fields.size() > 1 && fields[0] == "Host")
            {
                for (int i = 1; i < fields.size(); ++i)
                    if (!(fields[i].contains('*') || fields[i].contains('?')))
                        hosts << fields[i];
            }
            else if (fields.size() > 1 && fields[0] == "Include")
            {
                hosts.unite(parseConfigFile((fields[1][0] == '~') ? QDir::home().filePath(fields[1]) : fields[1]));
            }
        }
        file.close();
    }
    return hosts;
}

Plugin::Plugin():
    apps(registry(), "applications"),
    tr_desc(tr("Configured SSH host – %1")),
    tr_conn(tr("Connect"))
{
    hosts.unite(parseConfigFile(QStringLiteral("/etc/ssh/config")));
    hosts.unite(parseConfigFile(QDir::home().filePath(".ssh/config")));
    INFO << QStringLiteral("Found %1 ssh hosts.").arg(hosts.size());
}

QString Plugin::synopsis() const
{ return tr("[user@]<host> [params…]"); }

bool Plugin::allowTriggerRemap() const
{ return false; }

std::vector<RankItem> Plugin::getItems(const QString &query, bool allowParams) const
{
    vector<RankItem> r;

    auto match = regex_synopsis.match(query);
    if (!match.hasMatch())
        return r;

    const auto q_user = match.captured(1);
    const auto q_host = match.captured(2);
    const auto q_params = match.captured(3);

    if (!(allowParams || q_params.isEmpty()))
        return r;

    for (const auto &host : hosts)
    {
        if (host.startsWith(q_host, Qt::CaseInsensitive))
        {
            QString cmd = "ssh ";
            if (!q_user.isEmpty())
                cmd += q_user + '@';
            cmd += host;
            if (!q_params.isEmpty())
                cmd += ' ' + q_params;

            auto a = [cmd, this]{ apps->runTerminal(QString("%1 || exec $SHELL").arg(cmd)); };

            r.emplace_back(
                StandardItem::make(host, host, tr_desc.arg(cmd), cmd, icon_urls, {{"c", tr_conn, a}}),
                (double)q_host.size() / host.size()
            );
        }
    }

    return r;
}


void Plugin::handleTriggerQuery(albert::Query *query)
{
    auto r = getItems(query->string(), true);
    applyUsageScore(&r);
    for (const auto &[i, s] : r)
        query->add(i);
}

vector<RankItem> Plugin::handleGlobalQuery(const Query *query) const
{
    return getItems(query->string(), false);
}

QWidget *Plugin::buildConfigWidget()
{
    auto *w = new QLabel(tr(
        "Provides session launch action items for host patterns in the "
        "SSH config that do not contain globbing characters."
    ));
    w->setAlignment(Qt::AlignTop);
    w->setWordWrap(true);
    return w;
}
