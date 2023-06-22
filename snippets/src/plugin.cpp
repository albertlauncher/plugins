// Copyright (c) 2023 Manuel Schneider

#include "plugin.h"
#include "ui_configwidget.h"
#include <QFileSystemModel>
#include <QMessageBox>
#include <QInputDialog>
#include <QTextStream>
#include <memory>
ALBERT_LOGGING
using namespace std;
using namespace albert;

struct SnippetItem : Item
{
    SnippetItem(const QFileInfo& fi, Plugin *p) : file_name_(fi.fileName()), plugin_(p) {}
    QString id() const override { return file_name_; }
    QString text() const override { return file_name_; }
    QString subtext() const override { return QString("Text snippet '%1'").arg(file_name_); }
    QStringList iconUrls() const override { return {":snippet"}; }
    vector<Action> actions() const override {
        return {
            {
                "copy", "Copy snippet to clipboard",
                [this](){
                    QFile f(QDir(plugin_->snippets_path).filePath(file_name_));
                    f.open(QIODevice::ReadOnly);
                    setClipboardText(QTextStream(&f).readAll());
                }
            },
            {
                "open", "Open snippet",
                [this]() { openUrl("file://" + QDir(plugin_->snippets_path).filePath(file_name_)); }
            },
            {
                "remove", "Remove snippet",
                [this]() { plugin_->removeSnippet(file_name_); }
            }
        };
    }

    const QString file_name_;
    Plugin * const plugin_;
};


Plugin::Plugin(): snippets_path(configDir().path())
{
    fs_watcher.addPath(snippets_path);
    connect(&fs_watcher, &QFileSystemWatcher::directoryChanged, this, [this](){updateIndexItems();});

    indexer.parallel = [this](const bool &abort){
        vector<IndexItem> r;
        for (const auto &f : configDir().entryInfoList(QDir::Files)){
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
            QFile file(QDir(snippets_path).filePath(text));
            file.open(QIODevice::WriteOnly);
            openUrl("file://" + file.fileName());
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

QWidget *Plugin::buildConfigWidget()
{
    auto *w = new QWidget;
    Ui::ConfigWidget ui;
    ui.setupUi(w);

    auto *model = new QFileSystemModel;
    connect(w, &QObject::destroyed, model, &QObject::deleteLater);
    model->setRootPath(snippets_path);
    model->setFilter(QDir::Files);
    model->setReadOnly(false);
    ui.listView->setModel(model);
    ui.listView->setRootIndex(model->index(snippets_path));

    connect(ui.listView, &QListView::activated, this,
            [model](const QModelIndex &index){ openUrl(QString("file://%1").arg(model->filePath(index))); });

    connect(ui.pushButton_opendir, &QPushButton::clicked, this,
            [this](){ openUrl(QString("file://%1").arg(snippets_path)); });

    connect(ui.pushButton_add, &QPushButton::clicked,
             this, [this, w=w](){ newSnippet(w); });

    connect(ui.pushButton_remove, &QPushButton::clicked, this,
            [this, model, lw=ui.listView](){
        if (lw->currentIndex().isValid())
            removeSnippet(model->filePath(lw->currentIndex()));
    });

    return w;
}
