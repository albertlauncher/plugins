// Copyright (c) 2022-2024 Manuel Schneider

#include "plugin.h"
#include <QWidget>
#include <QComboBox>
#include <QVBoxLayout>
#include <QLabel>
#include <albert/logging.h>
using namespace std;
ALBERT_LOGGING_CATEGORY("apps")

static const char* CFG_TERM = "terminal";

QString Plugin::defaultTrigger() const { return QStringLiteral("apps "); }

void Plugin::commonInitialize(unique_ptr<QSettings> &s)
{
    fs_watcher_.addPaths(appDirectories());
    connect(&fs_watcher_, &QFileSystemWatcher::directoryChanged, this, &Plugin::updateIndexItems);

    restore_use_non_localized_name(s);
    connect(this, &Plugin::use_non_localized_name_changed, this, &Plugin::updateIndexItems);
}

void Plugin::setUserTerminalFromConfig()
{
    if (terminals.empty())
    {
        WARN << "No terminals available.";
        user_terminal.reset();
    }
    else if (auto s = settings(); !s->contains(CFG_TERM))  // unconfigured
    {
        user_terminal = *terminals.begin();  // guaranteed to exist since not empty
        WARN << QString("No terminal configured. Using %1.").arg(user_terminal->name());
    }
    else  // user configured
    {
        auto term_id = s->value(CFG_TERM).toString();
        auto term_it = ranges::find_if(terminals, [&](const auto &t){ return t->id() == term_id; });
        if (term_it != terminals.end())
            user_terminal = *term_it;
        else
        {
            user_terminal = *terminals.begin();  // guaranteed to exist since not empty
            WARN << QString("Configured terminal '%1'  does not exist. Using %2.")
                        .arg(term_id, user_terminal->id());
        }
    }
}

QWidget *Plugin::createTerminalFormWidget()
{
    auto *w = new QWidget();
    auto *cb = new QComboBox;
    auto *l = new QVBoxLayout;
    auto *lbl = new QLabel;

    for (const auto &t : terminals)
    {
        cb->addItem(t->name(), t->id());
        if (t->id() == user_terminal->id())  // is current
            cb->setCurrentIndex(cb->count()-1);
    }

    connect(cb, qOverload<int>(&QComboBox::currentIndexChanged), this, [this, cb](int index)
    {
        auto term_id = cb->itemData(index).toString();
        if (auto it = ranges::find_if(terminals, [&](const auto &t){ return t->id() == term_id; });
            it != terminals.end())
        {
            user_terminal = *it;
            settings()->setValue(CFG_TERM, term_id);
        }
        else
            WARN << QString("Selected terminal '%1' vanished.").arg(term_id);
    });

    QString t = "https://github.com/albertlauncher/albert/issues/new"
                "?assignees=ManuelSchneid3r&title=Terminal+[terminal-name]+missing"
                "&body=Post+an+xterm+-e+compatible+commandline.";
    t = tr(R"(Report missing terminals <a href="%1">here</a>.)").arg(t);
    t = QString(R"(<span style="font-size:9pt; color:#808080;">%1</span>)").arg(t);
    lbl->setText(t);
    lbl->setOpenExternalLinks(true);

    l->addWidget(cb);
    l->addWidget(lbl);
    w->setLayout(l);
    return w;
}

