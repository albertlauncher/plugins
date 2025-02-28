// Copyright (c) 2023-2024 Manuel Schneider

#include "plugin.h"
#include "ui_configwidget.h"
#include <QFile>
#include <QFileSystemModel>
#include <QInputDialog>
#include <QMessageBox>
#include <QTextStream>
#include <albert/albert.h>
#include <albert/standarditem.h>
#include <memory>
ALBERT_LOGGING_CATEGORY("snippets")
using namespace albert;
using namespace std;

static const int preview_max_size = 100;

struct SnippetItem : Item
{
    SnippetItem(const QFileInfo& fi, Plugin *p)
        : file_base_name_(fi.completeBaseName()), plugin_(p)
    {
        QFile file(fi.filePath());
        if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QTextStream in(&file);
            preview_ = in.readAll().simplified();
            if (preview_.size() > preview_max_size)
                preview_ = preview_.left(preview_max_size) + " …";
            preview_.squeeze();
            file.close();
        } else
            WARN << "Failed to read from snippet file" << path();
    }

    QString id() const override
    { return file_base_name_; }

    QString text() const override
    { return file_base_name_; }

    QString subtext() const override
    {
        static const auto tr = QCoreApplication::translate("SnippetItem", "Text snippet");
        return QString("%1 – %2").arg(tr, preview_);
    }

    QStringList iconUrls() const override
    { return {":snippet"}; }

    vector<Action> actions() const override
    {
        static const auto tr_cp = QCoreApplication::translate("SnippetItem", "Copy and paste");
        static const auto tr_c = QCoreApplication::translate("SnippetItem", "Copy");
        static const auto tr_e = QCoreApplication::translate("SnippetItem", "Edit");
        static const auto tr_r = QCoreApplication::translate("SnippetItem", "Remove");

        vector<Action> actions;

        if (havePasteSupport())
            actions.emplace_back(
                "cp", tr_cp,
                [this]{
                    QFile f(path());
                    f.open(QIODevice::ReadOnly);
                    setClipboardTextAndPaste(QTextStream(&f).readAll());
                }
            );

        actions.emplace_back(
            "c", tr_c,
            [this]{
                QFile f(path());
                f.open(QIODevice::ReadOnly);
                setClipboardText(QTextStream(&f).readAll());
            }
        );

        actions.emplace_back("o", tr_e, [this]{ open(path()); });

        actions.emplace_back("r", tr_r, [this]{ plugin_->removeSnippet(file_base_name_+".txt"); });

        return actions;
    }

    QString path() const
    { return QDir(plugin_->configLocation()).filePath(file_base_name_ + ".txt"); }

private:

    const QString file_base_name_;
    QString preview_;
    Plugin * const plugin_;
};


Plugin::Plugin()
{
    const auto conf_path = configLocation();

    tryCreateDirectory(conf_path);

    fs_watcher.addPath(conf_path.c_str());
    connect(&fs_watcher, &QFileSystemWatcher::directoryChanged,
            this, [this]{ updateIndexItems(); });

    indexer.parallel = [this](const bool &abort){
        vector<IndexItem> r;
        for (const auto &f : QDir(configLocation()).entryInfoList({"*.txt"}, QDir::Files)){
            if (abort) return r;
            r.emplace_back(make_shared<SnippetItem>(f, this), f.completeBaseName());
        }
        return r;
    };
    indexer.finish = [this](vector<IndexItem> &&results){
        setIndexItems(::move(results));
    };
}

QString Plugin::defaultTrigger() const
{ return "snip "; }

QString Plugin::synopsis(const QString &) const
{
    static const auto tr_s = tr("<filter>|+");
    return tr_s;
}

void Plugin::updateIndexItems()
{ indexer.run(); }

void Plugin::handleTriggerQuery(Query &query)
{
    if (query.string() == QStringLiteral("+"))
        query.add(
            StandardItem::make(
                "+",
                tr("Create new snippet"),
                tr("Create snippet file and open it in default editor."),
                {":snippet"},
                {
                    {"add", tr("Create"), [this](){ addSnippet(); }}
                }
            )
        );
    else
        IndexQueryHandler::handleTriggerQuery(query);
}

void Plugin::addSnippet(const QString &text, QWidget *parent) const
{
    QString name;
    while (true)
    {
        bool ok;
        name = QInputDialog::getText(parent, qApp->applicationDisplayName(),
                                     tr("Snippet name:"),
                                     QLineEdit::Normal, {}, &ok);

        if (!ok)
            break;

        if (name.isEmpty())
        {
            QMessageBox::warning(parent, qApp->applicationDisplayName(),
                                 tr("The snippet name must not be empty."));
            continue;
        }

        QFile file(QDir(configLocation()).filePath(name) + ".txt");
        if (file.exists())
        {
            QMessageBox::warning(parent, qApp->applicationDisplayName(),
                                 tr("There is already a snippet called '%1'.").arg(name));
            continue;
        }

        if(file.open(QIODevice::WriteOnly))
        {
            if (text.isEmpty())
                open(file.fileName());
            else
                QTextStream(&file) << text;
            file.close();
        }
        else
            QMessageBox::critical(parent, qApp->applicationDisplayName(),
                                  tr("Failed creating the snippet file '%1'.")
                                      .arg(file.fileName()));

        break;
    }
}

void Plugin::removeSnippet(const QString &file_name) const
{
    auto path = QDir(configLocation()).filePath(file_name);
    if (!QFile::exists(path))
        WARN << "Path to remove does not exist:" << path;
    else if (QMessageBox::question(nullptr, qApp->applicationName(),
                                   tr("Move snippet '%1' to trash?").arg(file_name)) == QMessageBox::Yes)
        if (!QFile::moveToTrash(path))
            QMessageBox::warning(nullptr, qApp->applicationName(),
                                 tr("Failed to move snippet file to trash."));
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
    model->setRootPath(configLocation().c_str());

    ui.listView->setModel(model);
    ui.listView->setRootIndex(model->index(model->rootPath()));

    connect(ui.listView, &QListView::activated, this,
            [model](const QModelIndex &index){ open(model->filePath(index)); });

    connect(ui.pushButton_opendir, &QPushButton::clicked, this,
            [this](){ open(configLocation()); });

    connect(ui.pushButton_add, &QPushButton::clicked,
            this, [this, w=w](){ addSnippet({}, w); });

    connect(ui.pushButton_remove, &QPushButton::clicked, this,
            [this, model, lw=ui.listView](){
        if (lw->currentIndex().isValid())
            removeSnippet(model->filePath(lw->currentIndex()));
    });

    return w;
}
