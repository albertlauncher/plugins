// Copyright (c) 2023 Manuel Schneider

#include "albert/albert.h"
#include "albert/extension/queryhandler/item.h"
#include "albert/extension/queryhandler/standarditem.h"
#include "plugin.h"
#include "ui_configwidget.h"
#include <QFileSystemModel>
#include <QInputDialog>
#include <QMessageBox>
#include <QTextStream>
#include <memory>
ALBERT_LOGGING_CATEGORY("snippets")
using namespace albert;
using namespace std;

struct SnippetItem : Item
{
    SnippetItem(const QFileInfo& fi, Plugin *p) : file_base_name_(fi.completeBaseName()), plugin_(p) {}
    QString id() const override { return file_base_name_; }
    QString text() const override { return file_base_name_; }
    QString subtext() const override { return QString("Text snippet '%1'").arg(file_base_name_); }
    QStringList iconUrls() const override { return {":snippet"}; }
    vector<Action> actions() const override {
        return {
            {
                "copy", "Copy and paste snippet",
                [this](){
                    QFile f(QDir(plugin_->snippets_path).filePath(file_base_name_+".txt"));
                    f.open(QIODevice::ReadOnly);
                    setClipboardTextAndPaste(QTextStream(&f).readAll());
                }
            },
            {
                "copy", "Copy snippet",
                [this](){
                    QFile f(QDir(plugin_->snippets_path).filePath(file_base_name_+".txt"));
                    f.open(QIODevice::ReadOnly);
                    setClipboardText(QTextStream(&f).readAll());
                }
            },
            {
                "open", "Open snippet file",
                [this]() { openUrl(QUrl::fromLocalFile(QDir(plugin_->snippets_path).filePath(file_base_name_+".txt"))); }
            },
            {
                "remove", "Remove snippet file",
                [this]() { plugin_->removeSnippet(file_base_name_+".txt"); }
            }
        };
    }

    const QString file_base_name_;
    Plugin * const plugin_;
};


Plugin::Plugin(): snippets_path(configDir()->path())
{
    // Todo remove in future
    QDir snippets_dir(snippets_path);
    for(const auto &fn : snippets_dir.entryList(QDir::Files))
        if (!fn.endsWith(".txt"))
            snippets_dir.rename(fn, fn + ".txt");

    fs_watcher.addPath(snippets_path);
    connect(&fs_watcher, &QFileSystemWatcher::directoryChanged, this, [this](){updateIndexItems();});

    indexer.parallel = [this](const bool &abort){
        vector<IndexItem> r;
        for (const auto &f : configDir()->entryInfoList({"*.txt"}, QDir::Files)){
            if (abort) return r;
            r.emplace_back(make_shared<SnippetItem>(f, this), f.fileName());
        }
        return r;
    };
    indexer.finish = [this](vector<IndexItem> &&results){
        setIndexItems(::move(results));
    };
}

QString Plugin::defaultTrigger() const { return "snip "; }

QString Plugin::synopsis() const { return "<filter>|[add]"; }

void Plugin::updateIndexItems() { indexer.run(); }

void Plugin::handleTriggerQuery(TriggerQuery *query) const
{
    if (query->string() == "add")
        query->add(
            StandardItem::make(
                "snip-add",
                "Add new snippet",
                "Create snippet file and open it in default editor.",
                {":snippet"},
                {
                    {"add", "Add snippet", [this](){ newSnippet(nullptr); }}
                }
            )
        );
    else
        IndexQueryHandler::handleTriggerQuery(query);
}

void Plugin::newSnippet(QWidget *parent) const
{
    QString text = QInputDialog::getText(parent, qApp->applicationName(),
                                         "Enter the snippet name");
    if (!text.isNull()){
        if (!text.isEmpty()){
            QFile file(QDir(snippets_path).filePath(text) + ".txt");
            file.open(QIODevice::WriteOnly);
            openUrl(QUrl::fromLocalFile(file.fileName()));
        } else
            QMessageBox::information(parent, qApp->applicationName(),
                                     "The snippet name must not be empty.");
    }
}

void Plugin::removeSnippet(const QString &file_name) const
{
    auto path = QDir(snippets_path).filePath(file_name);
    if (!QFile::exists(path))
        WARN << "Path to remove does not exist:" << path;
    else if (QMessageBox::question(nullptr, qApp->applicationName(),
                                   QString("Move snippet '%1' to trash?").arg(file_name)) == QMessageBox::Yes)
        if (!QFile::moveToTrash(path))
            QMessageBox::warning(nullptr, qApp->applicationName(),
                                 "Failed to move snippet file to trash.");
}

class RedIfNotTxtFileSystemModel : public QFileSystemModel
{
public:
    RedIfNotTxtFileSystemModel(QObject *parent) : QFileSystemModel(parent){}
    virtual QVariant data(const QModelIndex & index, int role = Qt::DisplayRole) const {
        if (role == Qt::ForegroundRole && !index.data().toString().endsWith(".txt"))
            return QColorConstants::Red;
        else
            return QFileSystemModel::data(index, role);
    }
};


QWidget *Plugin::buildConfigWidget()
{
    auto *w = new QWidget;
    Ui::ConfigWidget ui;
    ui.setupUi(w);

    auto *model = new RedIfNotTxtFileSystemModel(ui.listView);
    model->setFilter(QDir::Files);
    model->setReadOnly(false);
    model->setRootPath(snippets_path);

    ui.listView->setModel(model);
    ui.listView->setRootIndex(model->index(snippets_path));

    connect(ui.listView, &QListView::activated, this,
            [model](const QModelIndex &index){ openUrl(QUrl::fromLocalFile(model->filePath(index))); });

    connect(ui.pushButton_opendir, &QPushButton::clicked, this,
            [this](){ openUrl(QUrl::fromLocalFile(snippets_path)); });

    connect(ui.pushButton_add, &QPushButton::clicked,
             this, [this, w=w](){ newSnippet(w); });

    connect(ui.pushButton_remove, &QPushButton::clicked, this,
            [this, model, lw=ui.listView](){
        if (lw->currentIndex().isValid())
            removeSnippet(model->filePath(lw->currentIndex()));
    });

    return w;
}
