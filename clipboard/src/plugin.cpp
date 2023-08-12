// Copyright (c) 2022 Manuel Schneider

#include "albert/albert.h"
#include "albert/extension/queryhandler/standarditem.h"
#include "albert/logging.h"
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


Plugin::Plugin() : clipboard(QGuiApplication::clipboard())
{
    auto s = settings();
    persistent = s->value(CFG_PERSISTENCE, DEF_PERSISTENCE).toBool();
    length = s->value(CFG_HISTORY_LENGTH, DEF_HISTORY_LENGTH).toUInt();
    if (persistent)
        readHistory();

    connect(&timer, &QTimer::timeout, this, [this](){
        auto text = clipboard->text();

        // skip if …
        if (text.isEmpty() // empty text (images, pixmaps etc)
            || text.trimmed().isEmpty()  // spaces only
            || (history.size() != 0 && history.crbegin()->text == text)) // unchanged
            return;

        // remove dups
        history.erase(std::remove_if(history.begin(), history.end(),
                                     [&text](const ClipboardEntry &entry) { return entry.text == text; }),
                      history.end());

        // add an entry
        history.emplace_back(text, QDateTime::currentDateTime());

        // adjust lenght
        while (length < history.size())
            history.pop_front();

        // store
        if (persistent)
            writeHistory();
    });

    timer.start(500);
}

void Plugin::handleTriggerQuery(TriggerQuery *query) const
{
    auto trimmed = query->string().trimmed();
    QLocale loc;
    int rank = 1;
    for (auto it = history.crbegin(); it != history.crend(); ++it, ++rank){
        const auto &entry = *it;
        if (it->text.contains(trimmed))
            query->add(
                StandardItem::make(
                    id(),
                    entry.text,
                    QString("#%1 %2").arg(rank).arg(loc.toString(entry.datetime, QLocale::LongFormat)),
                    {":clipboard"},
                    {
                        {
                            "copy", "Copy and paste snippet",
                            [t=entry.text](){ setClipboardTextAndPaste(t); }
                        }, {
                            "copy", "Copy to clipboard",
                            [t=entry.text](){ setClipboardText(t); }
                        }, {
                            "rem", "Remove from history",
                            [this, t=entry.text](){
                                history.remove_if([t](const ClipboardEntry& ce){ return ce.text == t; });
                                if (persistent)
                                    writeHistory();
                            }
                        }
                    }
                )
            );
    }
}

QWidget *Plugin::buildConfigWidget()
{
    auto *w = new QWidget;
    auto *l = new QFormLayout;

    auto *c = new QCheckBox();
    c->setChecked(persistent);
    connect(c, &QCheckBox::toggled, this, [this](bool checked){
        persistent = checked;
        settings()->setValue(CFG_PERSISTENCE, persistent);
        if (persistent)
            writeHistory();
    });
    l->addRow("Persistent history", c);

    auto *s = new QSpinBox;
    s->setMinimum(1);
    s->setMaximum(10000000);
    s->setValue(length);
    connect(s, &QSpinBox::valueChanged, this, [this](int value){
        if ((uint)value < length){
            while ((uint)value < history.size())
                history.pop_front();
            writeHistory();
        }
        length = value;
        settings()->setValue(CFG_HISTORY_LENGTH, length);
    });
    l->addRow("History length", s);

    w->setLayout(l);
    return w;
}

void Plugin::writeHistory() const
{
    QFile file(dataDir()->filePath(HISTORY_FILE_NAME));

    DEBG << "Wrinting clipboard history to" << file.fileName();

    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)){
        WARN << "Failed writing to clipboard history.";
        QMessageBox::warning(nullptr, qApp->applicationName(),
                             "Failed writing to clipboard history.");
        return;
    }

    QJsonArray array;
    for (const auto &entry : history){
        QJsonObject object;
        object["text"] = entry.text;
        object["datetime"] = entry.datetime.toSecsSinceEpoch();
        array.append(object);
    }

    file.write(QJsonDocument(array).toJson());
    file.close();
}

void Plugin::readHistory()
{
    QFile file(dataDir()->filePath(HISTORY_FILE_NAME));
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)){
        WARN << "Failed reading from clipboard history.";
        QMessageBox::warning(nullptr, qApp->applicationName(),
                             "Failed reading from clipboard history.");
        return;
    }

    for (const auto &value : QJsonDocument::fromJson(file.readAll()).array()){
        const auto object = value.toObject();
        history.emplace_back(object["text"].toString(),
                             QDateTime::fromSecsSinceEpoch(object["datetime"].toInt()));
    }

    file.close();
}
