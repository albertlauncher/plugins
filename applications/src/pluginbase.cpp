// Copyright (c) 2022-2024 Manuel Schneider

#include "pluginbase.h"
#include "terminal.h"
#include <QCheckBox>
#include <QComboBox>
#include <QFormLayout>
#include <QLabel>
#include <QMessageBox>
#include <QVBoxLayout>
#include <QWidget>
#include <albert/iconprovider.h>
#include <albert/indexitem.h>
#include <albert/logging.h>
using namespace albert;
using namespace std;
ALBERT_LOGGING_CATEGORY("apps")

static const char* CFG_TERM = "terminal";

QString PluginBase::defaultTrigger() const { return QStringLiteral("apps "); }

void PluginBase::updateIndexItems()  { indexer.run(); }

void PluginBase::commonInitialize(unique_ptr<QSettings> &s)
{
    restore_use_non_localized_name(s);
    restore_split_camel_case(s);
    restore_use_acronyms(s);

    for (auto f : { &PluginBase::use_non_localized_name_changed,
                    &PluginBase::split_camel_case_changed,
                    &PluginBase::use_acronyms_changed })
        connect(this, f, this, &PluginBase::updateIndexItems);
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

    auto sorted_terminals = terminals;
    ranges::sort(sorted_terminals, [](const auto *t1, const auto *t2)
                 { return t1->name().compare(t2->name(), Qt::CaseInsensitive) < 0; });

    for (uint i = 0; i < sorted_terminals.size(); ++i)
    {
        const auto t = sorted_terminals.at(i);
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
    l->setContentsMargins(0,0,0,0);

    w->setLayout(l);

    return w;
}

void PluginBase::addBaseConfig(QFormLayout *l)
{
    auto *cb = new QCheckBox;
    l->addRow(tr("Use non-localized name"), cb);
    ALBERT_PROPERTY_CONNECT_CHECKBOX(this, use_non_localized_name, cb);

    cb = new QCheckBox;
    l->addRow(tr("Split CamelCase words (medial capital)"), cb);
    ALBERT_PROPERTY_CONNECT_CHECKBOX(this, split_camel_case, cb);

    cb = new QCheckBox;
    l->addRow(tr("Use acronyms"), cb);
    ALBERT_PROPERTY_CONNECT_CHECKBOX(this, use_acronyms, cb);

    l->addRow(tr("Terminal"), createTerminalFormWidget());
}

vector<IndexItem> PluginBase::buildIndexItems() const
{
    vector<IndexItem> r;

    for (const auto &iapp : applications)
    {
        auto app = static_pointer_cast<Application>(iapp);
        for (const auto &name : app->names())
        {
            r.emplace_back(app, name);

            // https://en.wikipedia.org/wiki/Combining_Diacritical_Marks
            static QRegularExpression re(R"([\x{0300}-\x{036f}])");
            auto normalized = name.normalized(QString::NormalizationForm_D).remove(re);

            auto ccs = camelCaseSplit(normalized);

            if (split_camel_case_)
                r.emplace_back(app, ccs.join(QChar::Space));

            if (use_acronyms_)
            {
                QString acronym;
                for (const auto &w : ccs)
                    if (w.size())
                        acronym.append(w[0]);

                if (acronym.size() > 1)
                    r.emplace_back(app, acronym);
            }
        }
    }

    return r;
}

QStringList PluginBase::camelCaseSplit(const QString &s)
{
    static QRegularExpression re(R"([A-Z0-9]?[a-z]+|[A-Z0-9]+(?![a-z]))");
    auto it = re.globalMatch(s);

    QStringList words;
    while (it.hasNext())
        words << it.next().captured();

    return words;
}

void PluginBase::runTerminal(const QString &script) const
{
    if (terminal)
        terminal->launch(script);
    else
        QMessageBox::warning(nullptr, {}, tr("No terminal available."));
}

