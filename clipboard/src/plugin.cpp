// Copyright (c) 2022-2024 Manuel Schneider

#include "plugin.h"
#include <QCheckBox>
#include <QDir>
#include <QFile>
#include <QFormLayout>
#include <QGuiApplication>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QMessageBox>
#include <QSettings>
#include <QSpinBox>
#include <albert/extensionregistry.h>
#include <albert/logging.h>
#include <albert/matcher.h>
#include <albert/plugin/snippets.h>
#include <albert/standarditem.h>
#include <albert/util.h>
#include <shared_mutex>
ALBERT_LOGGING_CATEGORY("clipboard")
using namespace albert;
using namespace std;

namespace {
static const char* HISTORY_FILE_NAME = "clipboard_history";
static const char* CFG_PERSISTENCE = "persistent";
static const bool DEF_PERSISTENCE = false;
static const char* CFG_HISTORY_LENGTH = "history_length";
static const uint DEF_HISTORY_LENGTH = 100;
}


Plugin::Plugin():
    clipboard(QGuiApplication::clipboard()),
    snippets(registry(), "snippets")
{
    // Load settings

    auto s = settings();
    persistent = s->value(CFG_PERSISTENCE, DEF_PERSISTENCE).toBool();
    length = s->value(CFG_HISTORY_LENGTH, DEF_HISTORY_LENGTH).toUInt();


    // Load history, if configured

    if (persistent)
    {
        if (QFile file(QDir(dataLocation()).filePath(HISTORY_FILE_NAME));
            file.open(QIODevice::ReadOnly | QIODevice::Text))
        {
            DEBG << "Reading clipboard history from" << file.fileName();
            const auto arr = QJsonDocument::fromJson(file.readAll()).array();
            for (const auto &value : arr)
            {
                const auto object = value.toObject();
                history.emplace_back(object["text"].toString(),
                                     QDateTime::fromSecsSinceEpoch(object["datetime"].toInt()));
            }
            file.close();
        }
        else
            DEBG << "Failed reading from clipboard history.";
    }


    // Init clipboard pull timer

    timer.start(500);
    connect(&timer, &QTimer::timeout, this, &Plugin::checkClipboard);
}

Plugin::~Plugin()
{
    if (persistent)
    {
        QJsonArray array;
        for (const auto &entry : history)
        {
            QJsonObject object;
            object["text"] = entry.text;
            object["datetime"] = entry.datetime.toSecsSinceEpoch();
            array.append(object);
        }

        QDir data_dir = dataLocation();
        if (data_dir.exists() || data_dir.mkpath("."))
        {
            if (QFile file(data_dir.filePath(HISTORY_FILE_NAME));
                file.open(QIODevice::WriteOnly))
            {
                DEBG << "Writing clipboard history to" << file.fileName();
                file.write(QJsonDocument(array).toJson());
                file.close();
            }
            else
                WARN << "Failed creating history file:" << data_dir.path();
        }
        else
            WARN << "Failed creating data dir" << data_dir.path();
    }
}

QString Plugin::defaultTrigger() const { return " "; }

void Plugin::handleTriggerQuery(Query *query)
{
    QLocale loc;
    int rank = 0;
    Matcher matcher(query->string());

    shared_lock l(mutex);

    for (const auto &entry : history)
    {
        ++rank;
        if (matcher.match(entry.text))
        {
            static const auto tr_cp = tr("Copy and paste");
            static const auto tr_c = tr("Copy");
            static const auto tr_r = tr("Remove");

            vector<Action> actions;

            if(havePasteSupport())
                actions.emplace_back(
                    "c", tr_cp,
                    [t=entry.text](){ setClipboardTextAndPaste(t); }
                );

            actions.emplace_back(
                "cp", tr_c,
                [t=entry.text](){ setClipboardText(t); }
            );

            actions.emplace_back(
                "r", tr_r,
                [this, t=entry.text]()
                {
                    lock_guard lock(mutex);
                    this->history.remove_if([t](const auto& ce){ return ce.text == t; });
                }
            );

            if (snippets)
                actions.emplace_back(
                    "s", tr("Save as snippet"),
                    [this, t=entry.text]()
                    {
                        snippets->addSnippet(t);
                    });

            query->add(
                StandardItem::make(
                    id(),
                    entry.text,
                    QString("#%1 %2").arg(rank).arg(loc.toString(entry.datetime, QLocale::LongFormat)),
                    {":clipboard"},
                    ::move(actions)
                )
            );
        }
    }
}

QWidget *Plugin::buildConfigWidget()
{
    auto *w = new QWidget;
    auto *l = new QFormLayout;

    auto *c = new QCheckBox();
    c->setChecked(persistent);
    c->setToolTip(tr("Stores the history on disk so that it persists across restarts."));
    l->addRow(tr("Store history"), c);
    connect(c, &QCheckBox::toggled, this, [this](bool checked)
            { settings()->setValue(CFG_PERSISTENCE, persistent = checked); });

    auto *s = new QSpinBox;
    s->setMinimum(1);
    s->setMaximum(10000000);
    s->setValue(length);
    l->addRow(tr("History length"), s);
    connect(s, &QSpinBox::valueChanged, this, [this](int value)
            {
                settings()->setValue(CFG_HISTORY_LENGTH, length = value);

                lock_guard lock(mutex);
                if (length < history.size())
                    history.resize(length);
            });

    w->setLayout(l);
    return w;
}

void Plugin::checkClipboard()
{
    // skip empty text (images, pixmaps etc), spaces only or no change
    if (auto text = clipboard->text();
        text.trimmed().isEmpty() || text == clipboard_text )
        return;
    else
        clipboard_text = text;

    lock_guard lock(mutex);

    // remove dups
    history.erase(remove_if(history.begin(), history.end(),
                            [this](const auto &ce) { return ce.text == clipboard_text; }),
                  history.end());

    // add an entry
    history.emplace_front(clipboard_text, QDateTime::currentDateTime());

    // adjust lenght
    if (length < history.size())
        history.resize(length);
}
