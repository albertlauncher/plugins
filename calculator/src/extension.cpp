// Copyright (C) 2014-2018 Manuel Schneider

#include <QClipboard>
#include <QLocale>
#include <QPointer>
#include <QSettings>
#include <QString>
#include <vector>
#include "albert/query.h"
#include "albert/util/standardactions.h"
#include "albert/util/standarditem.h"
#include "configwidget.h"
#include "extension.h"
#include "muParser.h"
#include "muParserInt.h"
#include "xdg/iconlookup.h"
Q_LOGGING_CATEGORY(qlc, "calculator")
#define DEBG qCDebug(qlc,).noquote()
#define INFO qCInfo(qlc,).noquote()
#define WARN qCWarning(qlc,).noquote()
#define CRIT qCCritical(qlc,).noquote()
using namespace std;
using namespace Core;
namespace {
const QString CFG_SEPS      = "group_separators";
const bool    CFG_SEPS_DEF  = false;
const QString CFG_HEXP      = "hex_parsing";
const bool    CFG_HEXP_DEF  = false;
}


struct Calculator::Extension::Private
{
    QPointer<ConfigWidget> widget;
    std::unique_ptr<mu::Parser> parser;
    std::unique_ptr<mu::ParserInt> iparser;
    QLocale locale;
    QString iconPath;
};


/** ***************************************************************************/
Calculator::Extension::Extension()
    : Core::Extension("org.albert.extension.calculator"),
      Core::QueryHandler(Core::Plugin::id()),
      d(new Private){

    registerQueryHandler(this);

    // FIXME Qt6 Workaround for https://bugreports.qt.io/browse/QTBUG-58504
    d->locale = QLocale(QLocale::system().name());

    QString iconPath = XDG::IconLookup::iconPath("calc");
    d->iconPath = iconPath.isNull() ? ":calc" : iconPath;

    d->parser.reset(new mu::Parser);
    d->parser->SetDecSep(d->locale.decimalPoint().toLatin1());
    d->parser->SetThousandsSep(d->locale.groupSeparator().toLatin1());
    d->parser->SetArgSep(';');

    if (settings().value(CFG_SEPS, CFG_SEPS_DEF).toBool())
        setGroupSeparatorEnabled(true);

    if (settings().value(CFG_HEXP, CFG_HEXP_DEF).toBool())
        setIParserEnabled(true);
}

/** ***************************************************************************/
Calculator::Extension::~Extension() { }


/** ***************************************************************************/
QWidget *Calculator::Extension::widget(QWidget *parent) {
    if (d->widget.isNull()){
        d->widget = new ConfigWidget(parent);

        d->widget->ui.checkBox_groupsep->setChecked(!(d->locale.numberOptions() & QLocale::OmitGroupSeparator));
        connect(d->widget->ui.checkBox_groupsep, &QCheckBox::toggled,
                [this](bool checked){ setGroupSeparatorEnabled(checked); });

        d->widget->ui.checkBox_hexparsing->setChecked(!(d->locale.numberOptions() & QLocale::OmitGroupSeparator));
        connect(d->widget->ui.checkBox_hexparsing, &QCheckBox::toggled,
                [this](bool checked){ setIParserEnabled(checked); });

    }
    return d->widget;
}


/** ***************************************************************************/
void Calculator::Extension::handleQuery(Core::Query * query) const {

    if (query->string().isEmpty())
        return;

    // http://beltoforion.de/article.php?a=muparser&p=errorhandling
    QString result;
    try {
        if(d->iparser && query->string().contains("0x")) {
            d->iparser->SetExpr(query->string().toStdString());
            result = d->locale.toString(d->iparser->Eval(), 'G', 16);
        } else {
            d->parser->SetExpr(query->string().toStdString());
            result = d->locale.toString(d->parser->Eval(), 'G', 16);
        }
    } catch (mu::Parser::exception_type &exception) {
        DEBG << "Muparser SetExpr exception: " << exception.GetMsg().c_str();
        return;
    }

    query->addMatch(makeStdItem("muparser",
        d->iconPath, result, QString("Result of '%1'").arg(query->string()),
        ActionList{
            makeClipAction("Copy result to clipboard", result),
            makeClipAction("Copy equation to clipboard", QString("%1 = %2").arg(query->string(), result))
        },
        result
    ), UINT_MAX);
}


/** ***************************************************************************/
void Calculator::Extension::setGroupSeparatorEnabled(bool enabled){
    settings().setValue(CFG_SEPS, enabled);
    if (enabled)
        d->locale.setNumberOptions(d->locale.numberOptions() & ~QLocale::OmitGroupSeparator);
    else
        d->locale.setNumberOptions(d->locale.numberOptions() | QLocale::OmitGroupSeparator);
}


/** ***************************************************************************/
void Calculator::Extension::setIParserEnabled(bool enabled){
    settings().setValue(CFG_HEXP, enabled);

    if (enabled){
        d->iparser.reset(new mu::ParserInt);
        d->iparser->SetDecSep(d->locale.decimalPoint().toLatin1());
        d->iparser->SetThousandsSep(d->locale.groupSeparator().toLatin1());
        d->iparser->SetArgSep(';');
    }
    else
        d->iparser.reset();
}
