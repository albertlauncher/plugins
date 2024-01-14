// Copyright (c) 2017-2024 Manuel Schneider

#include "albert/albert.h"
#include "albert/extension/queryhandler/standarditem.h"
#include "albert/logging.h"
#include "plugin.h"
#include <QDir>
#include <QFile>
#include <QLabel>
#include <QRegularExpression>
#include <QString>
#include <QTextStream>
#include <QWidget>
ALBERT_LOGGING_CATEGORY("ssh")
using namespace albert;
using namespace std;

const QStringList Plugin::icon_urls = {"xdg:ssh", ":ssh"};

const QRegularExpression Plugin::re_input = QRegularExpression(R"raw(^(?:(\w+)@)?\[?([\w\.-]*)\]?(?:\h+(.*))?$)raw");

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

Plugin::Plugin()
{
    hosts_.unite(parseConfigFile(QStringLiteral("/etc/ssh/config")));
    hosts_.unite(parseConfigFile(QDir::home().filePath(".ssh/config")));
    INFO << QStringLiteral("Found %1 ssh hosts.").arg(hosts_.size());
}

QString Plugin::synopsis() const
{ return "[user@]<host> [params…]"; }

vector<RankItem> Plugin::handleGlobalQuery(const GlobalQuery *query) const
{
    const auto &trimmed = query->string().trimmed();
    vector<RankItem> rank_items;

    auto match = re_input.match(query->string());
    if (!match.hasMatch())
        return rank_items;

    const auto q_user = match.captured(1);
    const auto q_host = match.captured(2);
    const auto q_params = match.captured(3);
    static const auto tr_desc = tr("Configured SSH host – ssh %1");
    static const auto tr_conn = tr("Connect");

    for (const auto &host : hosts_)
    {
        if (host.startsWith(q_host, Qt::CaseInsensitive))
        {
            auto args = QString("%1%2 %3")
                            .arg(q_user.isEmpty()
                                     ? QString()
                                     : QStringLiteral("%1@").arg(q_user),
                                 host, q_params);

            rank_items.emplace_back(
                StandardItem::make(
                    host,
                    host,
                    tr_desc.arg(args),
                    args,
                    icon_urls,
                    {
                        {
                            "c", tr_conn,
                            [args](){ runTerminal(QString("ssh %1 && exit").arg(args)); }
                        }
                    }
                ),
                (float)q_host.size() / host.size()
            );
        }
    }

    return rank_items;
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
