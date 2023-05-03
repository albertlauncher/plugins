// Copyright (c) 2022-2023 Manuel Schneider

#include "plugin.h"
#include "ui_configwidget.h"
#include <QString>
ALBERT_LOGGING
using namespace std;
using namespace albert;

namespace {
const char* CFG_SEPS     = "group_separators";
const bool  CFG_SEPS_DEF = false;
const char* CFG_HEXP     = "hex_parsing";
const bool  CFG_HEXP_DEF = false;
}

Plugin::Plugin()
{
    if (QString loc = qgetenv("LC_NUMERIC"); !loc.isEmpty())
        locale = QLocale(loc);
    else if (loc = qgetenv("LC_ALL"); !loc.isEmpty())
        locale = QLocale(loc);
    else
        locale = QLocale(QLocale::system().name());

    parser = make_unique<mu::Parser>();
    parser->SetDecSep(locale.decimalPoint()[0].toLatin1());
    parser->SetThousandsSep(locale.groupSeparator()[0].toLatin1());
    parser->SetArgSep(';');

    if (settings()->value(CFG_SEPS, CFG_SEPS_DEF).toBool())
        setGroupSeparatorEnabled(true);

    if (settings()->value(CFG_HEXP, CFG_HEXP_DEF).toBool())
        setIParserEnabled(true);
}

QWidget *Plugin::buildConfigWidget()
{
    auto widget = new QWidget;
    Ui::ConfigWidget ui;
    ui.setupUi(widget);

    ui.checkBox_groupsep->setChecked(!(locale.numberOptions() & QLocale::OmitGroupSeparator));
    connect(ui.checkBox_groupsep, &QCheckBox::toggled,
            this, &Plugin::setGroupSeparatorEnabled);

    ui.checkBox_hexparsing->setChecked(!(locale.numberOptions() & QLocale::OmitGroupSeparator));
    connect(ui.checkBox_hexparsing, &QCheckBox::toggled,
            this, &Plugin::setIParserEnabled);

    return widget;
}

QString Plugin::defaultTrigger() const { return "="; }

QString Plugin::synopsis() const { return "<math expression>"; }

vector<RankItem> Plugin::handleGlobalQuery(const GlobalQuery &query) const
{
    if (query.string().isEmpty())
        return {};

    // http://beltoforion.de/article.php?a=muparser&p=errorhandling
    QString result;
    try {
        if(iparser && query.string().contains("0x")) {
            iparser->SetExpr(query.string().toStdString());
            result = locale.toString(iparser->Eval(), 'G', 16);
        } else {
            parser->SetExpr(query.string().toStdString());
            result = locale.toString(parser->Eval(), 'G', 16);
        }
    } catch (mu::Parser::exception_type &exception) {
        return {};
    }

    vector<RankItem> items;
    items.emplace_back(StandardItem::make(
        "muparser",
        result,
        QString("Result of '%1'").arg(query.string()),
        result,
        {"xdg:calc", ":calc"},
        {
                {"cp-res", "Copy result to clipboard",
                 [=](){ setClipboardText(result); }},
                {"cp-equ", "Copy equation to clipboard",
                 [=, q=query.string()](){ setClipboardText(QString("%1 = %2").arg(q, result)); }}
        })
        , RankItem::MAX_SCORE);
    return items;
}


void Plugin::setGroupSeparatorEnabled(bool enabled)
{
    settings()->setValue(CFG_SEPS, enabled);
    if (enabled)
        locale.setNumberOptions(locale.numberOptions() & ~QLocale::OmitGroupSeparator);
    else
        locale.setNumberOptions(locale.numberOptions() | QLocale::OmitGroupSeparator);
}


void Plugin::setIParserEnabled(bool enabled)
{
    settings()->setValue(CFG_HEXP, enabled);

    if (enabled){
        iparser = make_unique<mu::ParserInt>();
        iparser->SetDecSep(locale.decimalPoint()[0].toLatin1());
        iparser->SetThousandsSep(locale.groupSeparator()[0].toLatin1());
        iparser->SetArgSep(';');
    }
    else
        iparser.reset();
}
