// Copyright (c) 2022 Manuel Schneider

#include "plugin.h"
#include "ui_configwidget.h"
#include <QString>
ALBERT_LOGGING
using namespace std;

namespace {
const char* CFG_SEPS     = "group_separators";
const bool  CFG_SEPS_DEF = false;
const char* CFG_HEXP     = "hex_parsing";
const bool  CFG_HEXP_DEF = false;
}

Plugin::Plugin()
{
    // FIXME Qt6 Workaround for https://bugreports.qt.io/browse/QTBUG-58504
    locale = QLocale(QLocale::system().name());

    parser = std::make_unique<mu::Parser>();
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


std::vector<albert::RankItem> Plugin::rankItems(const QString &string, const bool& isValid) const 
{
    if (string.isEmpty())
        return {};

    // http://beltoforion.de/article.php?a=muparser&p=errorhandling
    QString result;
    try {
        if(iparser && string.contains("0x")) {
            iparser->SetExpr(string.toStdString());
            result = locale.toString(iparser->Eval(), 'G', 16);
        } else {
            parser->SetExpr(string.toStdString());
            result = locale.toString(parser->Eval(), 'G', 16);
        }
    } catch (mu::Parser::exception_type &exception) {
        return {};
    }

    std::vector<albert::RankItem> items;
    items.emplace_back(albert::StandardItem::make(
        "muparser",
        result,
        QString("Result of '%1'").arg(string),
        {"xdg:calc", ":calc"},
        {
                {"cp-res", "Copy result to clipboard",
                 [=](){ albert::setClipboardText(result); }},
                {"cp-equ", "Copy equation to clipboard",
                 [=](){ albert::setClipboardText(QString("%1 = %2").arg(string, result)); }}
        })
    , albert::MAX_SCORE);
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
