// Copyright (c) 2022-2024 Manuel Schneider

#include "pluginbase.h"
#include "terminal.h"
#include <QComboBox>
#include <QLabel>
#include <QMessageBox>
#include <QVBoxLayout>
#include <QWidget>
#include <albert/iconprovider.h>
#include <albert/logging.h>
using namespace std;
ALBERT_LOGGING_CATEGORY("apps")

static const char* CFG_TERM = "terminal";

QString PluginBase::defaultTrigger() const { return QStringLiteral("apps "); }

void PluginBase::updateIndexItems()  { indexer.run(); }

void PluginBase::commonInitialize(unique_ptr<QSettings> &s)
{
    restore_use_non_localized_name(s);
    connect(this, &PluginBase::use_non_localized_name_changed,
            this, &PluginBase::updateIndexItems);
}

void PluginBase::setUserTerminalFromConfig()
{
    if (terminals.empty())
    {
        WARN << "No terminals available.";
        terminal = nullptr;
    }
    else if (auto s = settings(); !s->contains(CFG_TERM))  // unconfigured
    {
        terminal = *terminals.begin();  // guaranteed to exist since not empty
        WARN << QString("No terminal configured. Using %1.").arg(terminal->name());
    }
    else  // user configured
    {
        auto term_id = s->value(CFG_TERM).toString();
        auto term_it = ranges::find_if(terminals, [&](const auto *t){ return t->id() == term_id; });
        if (term_it != terminals.end())
            terminal = *term_it;
        else
        {
            terminal = *terminals.begin();  // guaranteed to exist since not empty
            WARN << QString("Configured terminal '%1'  does not exist. Using %2.")
                        .arg(term_id, terminal->id());
        }
    }
}

QWidget *PluginBase::createTerminalFormWidget()
{
    auto *w = new QWidget();
    auto *cb = new QComboBox;
    auto *l = new QVBoxLayout;
    auto *lbl = new QLabel;

    for (uint i = 0; i < terminals.size(); ++i)
    {
        const auto t = terminals.at(i);
        cb->addItem(albert::iconFromUrls(t->iconUrls()), t->name(), t->id());
        cb->setItemData(i, t->id(), Qt::ToolTipRole);
        if (t->id() == terminal->id())  // is current
            cb->setCurrentIndex(i);
    }

    // cb->addItem(QIcon(), tr("Custom command"));

    connect(cb, qOverload<int>(&QComboBox::currentIndexChanged), this, [this, cb](int index)
    {
        auto term_id = cb->itemData(index).toString();
        if (auto it = ranges::find_if(terminals, [&](const auto &t){ return t->id() == term_id; });
            it != terminals.end())
        {
            terminal = *it;
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

void PluginBase::runTerminal(const QString &script) const
{
    if (terminal)
        terminal->launch(script);
    else
        QMessageBox::warning(nullptr, {}, tr("No terminal available."));
}

