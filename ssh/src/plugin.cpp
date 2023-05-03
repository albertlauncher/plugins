// Copyright (c) 2022-2023 Manuel Schneider

#include "plugin.h"
#include "ui_configwidget.h"
#include <QDir>
#include <QFile>
#include <QRegularExpression>
#include <QString>
#include <QTextStream>
#include <QWidget>
#include <utility>
ALBERT_LOGGING
using namespace std;
using namespace albert;
static const char* CFG_USE_KNOWN_HOSTS = "use_known_hosts";
static const bool  DEF_USE_KNOWN_HOSTS = true;

static const QString system_conf_file_path = QString("/etc/ssh/config");
static const QString user_conf_file_path = QDir::home().filePath(".ssh/config");
static const QString known_hosts_file_path = QDir::home().filePath(".ssh/known_hosts");
static const QStringList icon_urls = {"xdg:ssh", ":ssh"};

static const QRegularExpression re_known_hosts(R"raw(^\[?([\w\-\.\:]+)\]?(?::(\d+))?)raw");
static const QRegularExpression re_input(R"raw(^(?:(\w+)@)?\[?((?:[\w\.-]*))\]?(?::(\d+))?(?:\s+(.*))?$)raw");

struct SshItem : Item
{
    explicit SshItem(QString h, QString p, QString i):
            host(std::move(h)), port(std::move(p)), info(std::move(i)) {}
    SshItem(const SshItem&) = default;

    QString user;
    QString host;
    QString port;
    QString cmdln;
    QString info;

    QString id() const override { return host; }
    QString text() const override {
        if (user.isEmpty()){
            if (port.isEmpty()){
                return host;
            } else
                return QString("%1:%2").arg(host, port);
        } else {
            if (port.isEmpty()){
                return QString("%1@%2").arg(user, host);
            } else
                return QString("%1@%2:%3").arg(user, host, port);
        }
    }
    QString subtext() const override { return QString("%1 - %2").arg(text(), info); }
    QStringList iconUrls() const override { return icon_urls; }
    void connect(bool close_term) const {
        auto authority = host;
        if (!user.isEmpty())
            authority = QString("%1@%2").arg(user, authority);
        if (!port.isEmpty())
            authority = QString("%1:%2").arg(authority, port);
        if (!cmdln.isEmpty())
            authority = QString("%1 %2").arg(authority, cmdln);
        runTerminal(QString("ssh %1").arg(authority), {}, close_term);
    }
    vector<Action> actions() const override {
        return {
            { "ssh-connect", "Connect", [this](){ connect(true); } },
            { "ssh-connect", "Connect (Keep terminal)", [this](){ connect(false); } }
        };
    }
};

static void getConfigHosts(vector<shared_ptr<SshItem>> &hosts, const bool &abort)
{
    std::function<void(const QString &)> scanfile = [&scanfile, &hosts, &abort](const QString &path) {
        if (QFile::exists(path)) {
            QFile file(path);
            if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
                QTextStream in(&file);
                while (!in.atEnd()) {
                    if (abort) return;
                    QStringList fields = in.readLine().split(" ", Qt::SkipEmptyParts);
                    if (fields.size() > 1 && fields[0] == "Host") {
                        for (int i = 1; i < fields.size(); ++i)
                            if (!(fields[i].contains('*') || fields[i].contains('?')))
                                hosts.emplace_back(new SshItem(fields[i], QString(), path));
                    } else if (fields.size() > 1 && fields[0] == "Include") {
                        // TODO move this somewhere else
                        // TODO make it proper tilde expansion
                        if (fields[1][0] == '~') {
                            fields[1].remove(0, 1).insert(0, QDir::homePath());
                        }
                        scanfile(fields[1]);
                    }
                }
                file.close();
            }
        }
    };

    // Get the hosts in config
    for (const QString &path: {system_conf_file_path, user_conf_file_path})
        if (!abort)
            scanfile(path);
}

static void getKnownHosts(vector<shared_ptr<SshItem>> &hosts, const bool &abort)
{
    map<QString, shared_ptr<SshItem>> unique_hosts;
    // Get the hosts in known_hosts
    if (QFile file(known_hosts_file_path); file.exists()) {
        if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QTextStream in(&file);
            while (!in.atEnd()) {
                if (abort) break;
                if (auto match = re_known_hosts.match(in.readLine()); match.hasMatch())
                    unique_hosts.emplace(match.captured(1), make_shared<SshItem>(match.captured(1),
                                                                                 match.captured(2),
                                                                                 known_hosts_file_path));
            }
            file.close();
        }
    }
    if (abort) return;
    for (auto &[hostname, item] : unique_hosts)
        hosts.emplace_back(::move(item));
}

Plugin::Plugin()
{
    indexer.parallel = [this](const bool &cancel){
        if (!fs_watcher_.files().isEmpty())
            fs_watcher_.removePaths(fs_watcher_.files());
        fs_watcher_.addPaths({system_conf_file_path, user_conf_file_path, known_hosts_file_path});
        vector<shared_ptr<SshItem>> hosts;
        getConfigHosts(hosts, cancel);
        if (useKnownHosts())
            getKnownHosts(hosts, cancel);
        return hosts;
    };
    indexer.finish = [this](vector<shared_ptr<SshItem>> && items){
        hosts_ = ::move(items);
        updateIndexItems();
    };

    connect(&fs_watcher_, &QFileSystemWatcher::fileChanged, this, [this](){ indexer.run(); });

    useKnownHosts_ = settings()->value(CFG_USE_KNOWN_HOSTS, DEF_USE_KNOWN_HOSTS).toBool();
    indexer.run();
}

void Plugin::updateIndexItems()
{
    vector<IndexItem> index_items;
    index_items.reserve(hosts_.size());
    for (const auto &host : hosts_)
        index_items.emplace_back(static_pointer_cast<Item>(host), host->host);
    setIndexItems(::move(index_items));
}

QWidget *Plugin::buildConfigWidget()
{
    auto *w = new QWidget();
    Ui::ConfigWidget ui;
    ui.setupUi(w);

    ui.checkBox_knownhosts->setChecked(useKnownHosts());
    connect(ui.checkBox_knownhosts, &QCheckBox::toggled,
            this, &Plugin::setUseKnownHosts);

    return w;
}

QString Plugin::synopsis() const
{
    return "[usr@]<hst>[:prt] [cmdln]";
}

void Plugin::handleTriggerQuery(TriggerQuery &query) const
{
    auto trimmed = query.string().trimmed();
    if (trimmed.isEmpty())
        handleGlobalQuery(dynamic_cast<GlobalQuery&>(query));
    else {
        // Check sanity of input
        QRegularExpressionMatch match = re_input.match(trimmed);

        if (match.hasMatch())
        {
            QString q_user = match.captured(1);
            QString q_host = match.captured(2);
            QString q_port = match.captured(3);
            QString q_cmdln = match.captured(4);

            struct GQ : public GlobalQuery {
                const QString &string_;
                const bool &valid_;
                GQ(const QString &s, const bool &v) : string_(s), valid_(v) {}
                const QString &string() const { return string_; }
                bool isValid() const { return valid_; }
            } gq(q_host, query.isValid());

            vector<RankItem> rank_items{IndexQueryHandler::handleGlobalQuery(gq)};
            sort(rank_items.begin(), rank_items.end(), [](const auto &a, const auto &b){ return a.score > b.score; });
            vector<shared_ptr<Item>> results;

            // Show all hosts matching the query
            for (auto &rank_item : rank_items) {
                auto item = make_shared<SshItem>(*static_pointer_cast<SshItem>(rank_item.item)); // copy
                if (!q_user.isEmpty()) item->user = q_user;
                if (!q_port.isEmpty()) item->port = q_port;
                if (!q_cmdln.isEmpty()) item->cmdln = q_cmdln;
                results.emplace_back(::move(item));
            }

            // Add the quick connect item
            if (!q_host.isEmpty()){
                auto *i = new SshItem(q_host, q_port, "Quick connect");
                i->user = q_user;
                i->cmdln = q_cmdln;
                results.emplace_back(i);
            }

            query.add(results);
        }
    }
}

bool Plugin::useKnownHosts() const
{
    return useKnownHosts_;
}


void Plugin::setUseKnownHosts(bool b)
{
    settings()->setValue(CFG_USE_KNOWN_HOSTS, b);
    useKnownHosts_ = b;
    indexer.run();
}







/*
https://regex101.com/

anme@host:888 comand
host:888 comand
anme@host comand
host comand
anme@host:888
host:888
anme@host
host
anme@[host]:888 comand
[host]:888 comand
anme@[host] comand
[host] comand
anme@[host]:888
[host]:888
anme@[host]
[host]

*/


