// Copyright (c) 2022 Manuel Schneider

#include "plugin.h"
#include "ui_configwidget.h"
#include <QFileSystemModel>
#include <QTextStream>
#include <memory>
ALBERT_LOGGING
using namespace std;

struct SnippetItem : albert::Item
{
    SnippetItem(const QFileInfo& fi) : file_name_(fi.fileName()) {}
    QString id() const override { return file_name_; }
    QString text() const override { return file_name_; }
    QString subtext() const override { return QString("Text snippet '%1'").arg(file_name_); }
    QStringList iconUrls() const override { return {":snippet"}; }
    vector<albert::Action> actions() const override {
        return {
            {"snip-copy-text", "Copy snippet to clipboard", [this](){
                QFile f(QDir(path).filePath(file_name_));
                f.open(QIODevice::ReadOnly);
                albert::setClipboardText(QTextStream(&f).readAll());
            }},
            {"snip-open", "Open snippet", [this]() {
                albert::openUrl("file://" + QDir(path).filePath(file_name_));
            }}
        };
    }

    const QString file_name_;
    static QString path;
};
QString SnippetItem::path;


Plugin::Plugin()
{
    SnippetItem::path = configDir().path();
    fs_watcher.addPath(SnippetItem::path);
    connect(&fs_watcher, &QFileSystemWatcher::directoryChanged, [this](){updateIndex();});
    updateIndex();
}

QString Plugin::defaultTrigger() const
{
    return "snip ";
}

vector<albert::IndexItem> Plugin::indexItems() const
{
    vector<albert::IndexItem> r;
    for (const auto &f : configDir().entryInfoList(QDir::Files))
        r.emplace_back(make_shared<SnippetItem>(f), f.fileName());
    return r;
}

QWidget *Plugin::buildConfigWidget()
{
    auto w = new QWidget;
    Ui::ConfigWidget ui;
    ui.setupUi(w);

    auto model = new QFileSystemModel();
    connect(w, &QObject::destroyed, model, &QObject::deleteLater);
    model->setRootPath(SnippetItem::path);
    model->setFilter(QDir::Files);
    model->setReadOnly(false);
    ui.listView->setModel(model);
    ui.listView->setRootIndex(model->index(SnippetItem::path));

    connect(ui.listView, &QListView::activated, [model](const QModelIndex &index){
        albert::openUrl(QString("file://%1").arg(model->filePath(index)));
    });
    connect(ui.pushButton_opendir, &QPushButton::clicked, [](){
        albert::openUrl(QString("file://%1").arg(SnippetItem::path));
    });

    return w;
}
