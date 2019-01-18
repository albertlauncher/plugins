// Copyright (C) 2014-2017 Manuel Schneider

#include <QDebug>
#include <QPointer>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <stdexcept>
#include <set>
#include <QFileSystemWatcher>
#include <QRegularExpression>
#include "albert/util/standarditem.h"
#include "albert/util/standardactions.h"
#include "xdg/iconlookup.h"
#include "configwidget.h"
#include "albert/util/offlineindex.h"
#include "extension.h"

using namespace Core;
using namespace std;

enum IDE_TYPE {
    VSCODE = 0,
    SUBLIME = 1,
    ATOM = 2,
};

namespace {

const char* CFG_FUZZY = "coder_fuzzy";
const bool  DEF_FUZZY = false;
const QString VSCODE_PROJS_DIR = QDir::home().filePath(".config/Code/storage.json");
const QString SUBLIME_PROJS_DIR = QDir::home().filePath(".config/sublime-text-3/Local/Session.sublime_session");

const QString DEF_VSCODE_CMD = "/usr/bin/code";
const QString DEF_SUBL_CMD = "/usr/bin/subl3";

}

static const QString getIcon(IDE_TYPE type) {
    if(type == VSCODE)
        return ":coder-vscode";
    else if(type == SUBLIME)
        return ":coder-sublime";
    else if(type == ATOM)
        return "";
    return "";
}

class Coder::ProjItem {
public:
    QString path;
    QString name;
    IDE_TYPE type;

    ProjItem(IDE_TYPE _type, const QString& _path, const QString& _name) {
        this->type = _type;
        this->path = _path;
        this->name = _name;
    }
};


class Coder::Private
{
public:
    QString icon;
    QPointer<ConfigWidget> widget;
    std::map<QString, ProjItem> projsMap;

    Core::OfflineIndex offlineIndex;
    QFileSystemWatcher fileSystemWatcher;

    QString sublCmds;
    QString codeCmds;

    void startIndexing();
    void finishIndexing();

    Private() {
        //w TODO: Maybe it can load from personal file.
        this->sublCmds = DEF_SUBL_CMD;
        this->codeCmds = DEF_VSCODE_CMD;
    }

    const QString getExecCommand(IDE_TYPE type);
};


const QString Coder::Private::getExecCommand(IDE_TYPE type) {
    if(type == VSCODE)
        return this->codeCmds;
    else if(type == SUBLIME)
        return this->sublCmds;
    else
        return "";
}

void Coder::Private::startIndexing() {

}

void Coder::Private::finishIndexing() {

}


/** ***************************************************************************/
Coder::Extension::Extension()
    : Core::Extension("org.albert.extension.coder"), // Must match the id in metadata
      Core::QueryHandler(Core::Plugin::id()),
      d(new Private) {

    registerQueryHandler(this);

    d->offlineIndex.setFuzzy(settings().value(CFG_FUZZY, DEF_FUZZY).toBool());
    connect(&d->fileSystemWatcher, &QFileSystemWatcher::fileChanged,
            this, &Coder::Extension::rescan);

    // You can throw in the constructor if something fatal happened,
    /*
    throw std::runtime_error( "Description of error." );
    throw std::string( "Description of error." );
    throw QString( "Description of error." );
    throw "Description of error.";
    throw; // Whatever prints "unknown error"
    */
    qInfo() << "In Coder init";
    rescan();
}



void getVSCodeProjs(const QString& path, std::map<QString, Coder::ProjItem>& retMap) {
    qDebug() << "Watch VSCode projs";
    QFile file(path);

    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QString val =file.readAll();
        file.close();
        QJsonDocument doc = QJsonDocument::fromJson(val.toUtf8());

        const QJsonObject& jsonObj = doc.object();

        const QJsonObject& jOpenedPathList = jsonObj["openedPathsList"].toObject();
        const QJsonArray& jWorkspaces = jOpenedPathList["workspaces2"].toArray();

        qDebug() << "[Coder]: The path size is " << jWorkspaces.size();
        QJsonArray::const_iterator it;
        for(it = jWorkspaces.constBegin(); it != jWorkspaces.constEnd(); it++) {
            QString absPath = it->toString();
            if(absPath.startsWith("file:///")) {
                absPath = absPath.mid(7);
            }
            QDir q(absPath);
            if(q.exists()) {
                qDebug() << "[Coder]: Get Dir name " << q.path();
                Coder::ProjItem item(IDE_TYPE::VSCODE, q.path(), q.dirName());
                retMap.insert(std::make_pair(q.dirName(), item));
            }
        }
    } else
        qWarning() << "Could not open the VSCode storage.json";
}


void getSublimeProjs(const QString& path, std::map<QString, Coder::ProjItem>& retMap) {

    QFile file(path);
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QString val =file.readAll();
        file.close();
        QJsonDocument doc = QJsonDocument::fromJson(val.toUtf8());

        const QJsonObject& jsonObj = doc.object();

        const QJsonArray& jWorkspaces = jsonObj["folder_history"].toArray();

        qDebug() << "[Coder]: The path size is " << jWorkspaces.size();
        QJsonArray::const_iterator it;
        for(it = jWorkspaces.constBegin(); it != jWorkspaces.constEnd(); it++) {
            QDir q(it->toString());
            Coder::ProjItem item(IDE_TYPE::SUBLIME, q.path(), q.dirName());
            retMap.insert(std::make_pair(q.dirName(), item));
        }
    } else
        qWarning() << "Could not open the sublime sublime_session";
}


/** ***************************************************************************/
Coder::Extension::~Extension() {

}



/** ***************************************************************************/
QWidget *Coder::Extension::widget(QWidget *parent) {
    if (d->widget.isNull()) {
        d->widget = new ConfigWidget(parent);
    }

    // Fuzzy
    d->widget->ui.checkBox_fuzzy->setChecked(fuzzy());
    connect(d->widget->ui.checkBox_fuzzy, &QCheckBox::toggled, this,
              &Extension::setFuzzy);

    // sublime command edit
    d->widget->ui.lineEdit_subpath->setText(d->sublCmds);
    connect(d->widget->ui.lineEdit_subpath, &QLineEdit::textEdited, [this](const QString &s) {
        d->sublCmds = s;
    });

    // vscode command edit
    d->widget->ui.lineEdit_vscodepath->setText(d->codeCmds);
    connect(d->widget->ui.lineEdit_vscodepath, &QLineEdit::textEdited, [this](const QString &s){
        d->codeCmds = s;
    });

    qDebug() << "Init with" << d->codeCmds << d->sublCmds;
    return d->widget;
}

bool Coder::Extension::fuzzy() {
    return d->offlineIndex.fuzzy();
}

void Coder::Extension::setFuzzy(bool b) {
    settings().setValue(CFG_FUZZY, b);
    d->offlineIndex.setFuzzy(b);

}


/** ***************************************************************************/
void Coder::Extension::setupSession() {

}



/** ***************************************************************************/
void Coder::Extension::teardownSession() {

}



/** ***************************************************************************/
void Coder::Extension::handleQuery(Core::Query *query) const {

    if (!query->string().trimmed().isEmpty() || query->isTriggered()) {
        QRegularExpression re(QString("(%1)").arg(query->string()),
                                  QRegularExpression::CaseInsensitiveOption);

        qDebug() << "In Search " << query->string();
        for(auto proj: d->projsMap) {
            const ProjItem& pItem = proj.second;

            if (pItem.name.startsWith(query->string()) ) {
                // create item
                auto item = std::make_shared<StandardItem>("vscode_" + pItem.path);
                qDebug() << pItem.name << " " << pItem.path;
                item->setText(QString(pItem.name).replace(re, "<u>\\1</u>"));
                item->setSubtext(QString("Open project %1").arg(pItem.path));
                item->setCompletion(QString("&c %1").arg(pItem.name));

                //TODO: because of std::move, this should use QString(getIcon())
                // https://stackoverflow.com/questions/28595117/why-can-we-use-stdmove-on-a-const-object
                item->setIconPath(getIcon(pItem.type));
                qDebug() << pItem.name << "with" << pItem.type << d->getExecCommand(pItem.type);
                item->addAction(
                   make_shared<ProcAction>(
                       QString("Open '%1' workspace").arg(proj.first),
                       QStringList() << d->getExecCommand(pItem.type)
                                     << pItem.path));
                query->addMatch(item);
            }
            // query->addMatch(item,
            // static_cast<uint>(1.0*query->string().size()/host.size()* UINT_MAX));
        }

        /*
        if ( query->isTriggered() ) {
            QString trimmed = query->string().trimmed();
            auto item = std::make_shared<StandardItem>();
            item->setText(trimmed);
            item->setSubtext()
        }
        */
        //TermAction;
    }

    /*
     * Things change so often I wont maintain this tutorial here. Check the relevant headers.
     *
     * - core/extension.h
     * - core/queryhandler.h
     * - core/query.h
     * - core/item.h
     * - core/action.h
     * - util/standarditem.h
     * - util/offlineindex.h
     * - util/standardindexitem.h
     *
     * Use
     *
     *   query->addMatch(my_item)
     *
     * to add matches. If you created a throw away item MOVE it instead of
     * copying e.g.:
     *
     *   query->addMatch(std::move(my_tmp_item))
     *
     * The relevance factor is optional. (Defaults to 0) its a usigned integer depicting the
     * relevance of the item 0 mean not relevant UINT_MAX is totally relevant (exact match).
     * E.g. it the query is "it" and your items name is "item"
     *
     *   my_item.name().startswith(query->string)
     *
     * is a naive match criterion and
     *
     *   UINT_MAX / ( query.searchterm().size() / my_item.name().size() )
     *
     * a naive match factor.
     *
     * If you have a lot of items use the iterator versions addMatches, e.g. like that
     *
     *   query->addMatches(my_items.begin(), my_items.end());
     *
     * If the items in the container are temporary object move them to avoid uneccesary
     * reference counting:
     *
     *   query->addMatches(std::make_move_iterator(my_tmp_items.begin()),
     *                     std::make_move_iterator(my_tmp_items.end()));
     */
}


void Coder::Extension::rescan() {
    qInfo() << "Start scan for coder";

    d->projsMap.clear();

    getVSCodeProjs(VSCODE_PROJS_DIR, d->projsMap);
    getSublimeProjs(SUBLIME_PROJS_DIR, d->projsMap);

    if (d->fileSystemWatcher.files().empty()) {
        if (!d->fileSystemWatcher.addPath(SUBLIME_PROJS_DIR))
            qWarning() << qPrintable(
                QString("%1 can not be watched. Changes in this path will not be noticed.").arg(SUBLIME_PROJS_DIR));

        if (!d->fileSystemWatcher.addPath(VSCODE_PROJS_DIR))
            qWarning() << qPrintable(
                QString("%1 can not be watched. Changes in this path will not be noticed.").arg(VSCODE_PROJS_DIR));
    }


    qInfo() << "Index" << d->projsMap.size() << "Coder projs";
}
