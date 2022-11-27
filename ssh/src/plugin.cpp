// Copyright (c) 2022 Manuel Schneider

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
static const char* CFG_USE_KNOWN_HOSTS = "use_known_hosts";
static const bool  DEF_USE_KNOWN_HOSTS = true;

static const QString system_conf_file_path = QString("/etc/ssh/config");
static const QString user_conf_file_path = QDir::home().filePath(".ssh/config");
static const QString known_hosts_file_path = QDir::home().filePath(".ssh/known_hosts");
static const QStringList icon_urls = {"xdg:ssh", ":ssh"};

struct SshItem : albert::Item
{
    explicit SshItem(QString host, QString port, QString from):
            host_(std::move(host)), port_(std::move(port)), from_file_(std::move(from)) {}
    SshItem(const SshItem&) = default;

    QString user_;
    QString host_;
    QString port_;
    QString from_file_;

    QString id() const override { return host_; }
    QString text() const override {
        if (user_.isEmpty()){
            if (port_.isEmpty()){
                return host_;
            } else
                return QString("%1:%2").arg(host_, port_);
        } else {
            if (port_.isEmpty()){
                return QString("%1@%2").arg(user_, host_);
            } else
                return QString("%1@%2:%3").arg(user_, host_, port_);
        }
    }
    QString subtext() const override { return QString("%1 (%2)").arg(text(), from_file_); }
    QStringList iconUrls() const override { return icon_urls; }
    vector<albert::Action> actions() const override { return {
                {"ssh-connect", "Connect to host", [this]() {
                    if (port_.isEmpty())
                        albert::runTerminal(QString("ssh %1").arg(host_));
                    else
                        albert::runTerminal(QString("ssh %1:%2").arg(host_, port_));
                }}
        }; }

};

static void getConfigHosts(vector<shared_ptr<SshItem>> &hosts, const bool &cancel)
{
    std::function<void(const QString &)> scanfile = [&scanfile, &hosts](const QString &path) {
        if (QFile::exists(path)) {
            QFile file(path);
            if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
                QTextStream in(&file);
                while (!in.atEnd()) {
                    QStringList fields = in.readLine().split(QRegularExpression("\\s+"), Qt::SkipEmptyParts);
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
        scanfile(path);
}

static void getKnownHosts(vector<shared_ptr<SshItem>> &hosts, const bool &cancel)
{
    map<QString, shared_ptr<SshItem>> unique_hosts;
    // Get the hosts in known_hosts
    if (QFile file(known_hosts_file_path); file.exists()) {
        if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QRegularExpression re(R"raw(^\[?([\w\-\.\:]+)\]?(?::(\d+))?)raw");
            QTextStream in(&file);
            while (!in.atEnd())
                if (auto match = re.match(in.readLine()); match.hasMatch())
                    unique_hosts.emplace(match.captured(1), make_shared<SshItem>(match.captured(1),
                                                                                 match.captured(2),
                                                                                 known_hosts_file_path));
            file.close();
        }
    }
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
        updateIndex();
    };

    connect(&fs_watcher_, &QFileSystemWatcher::fileChanged, [this](){ indexer.run(); });

    useKnownHosts_ = settings()->value(CFG_USE_KNOWN_HOSTS, DEF_USE_KNOWN_HOSTS).toBool();
    indexer.run();
}

vector<albert::IndexItem> Plugin::indexItems() const
{
    vector<albert::IndexItem> index_items;
    index_items.reserve(hosts_.size());
    for (const auto &host : hosts_)
        index_items.emplace_back(static_pointer_cast<albert::Item>(host), host->host_);
    return index_items;
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
    return "[user@]<host>[:port]";
}

void Plugin::handleQuery(Query &query) const
{
    auto trimmed = query.string().trimmed();
    if (trimmed.isEmpty())
        IndexQueryHandler::handleQuery(query);
    else {
        // Check sanity of input
        QRegularExpression re(R"raw(^(?:(\w+)@)?\[?((?:[\w\.-]*))\]?(?::(\d+))?$)raw");
        QRegularExpressionMatch match = re.match(trimmed);

        if (match.hasMatch())
        {
            QString q_user = match.captured(1);
            QString q_host = match.captured(2);
            QString q_port = match.captured(3);

            vector<albert::RankItem> rank_items{rankItems(q_host, query.isValid())};
            sort(rank_items.begin(), rank_items.end(), [](const auto &a, const auto &b){ return a.score > b.score; });
            vector<shared_ptr<albert::Item>> results;

            // Show all hosts matching the query
            for (auto &rank_item : rank_items) {
                auto item = make_shared<SshItem>(*static_pointer_cast<SshItem>(rank_item.item)); // copy
                if (!q_user.isEmpty()) item->user_ = q_user;
                if (!q_port.isEmpty()) item->port_ = q_port;
                results.emplace_back(::move(item));
            }

            // Add the quick connect item
            if (!q_host.isEmpty()){
                auto *i = new SshItem(q_host, q_port, "Quick connect");
                i->user_ = q_user;
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
