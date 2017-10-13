// Copyright (C) 2014-2017 Manuel Schneider

#include <QClipboard>
#include <QDebug>
#include <QLocale>
#include <QPointer>
#include <QSettings>
#include <vector>
#include "configwidget.h"
#include "extension.h"
#include "muParser.h"
#include "core/query.h"
#include "util/standarditem.h"
#include "util/standardactions.h"
#include "xdg/iconlookup.h"
using namespace std;
using namespace Core;



namespace {
const QString CFG_SEPS      = "group_separators";
const bool    CFG_SEPS_DEF  = false;
}



class Calculator::Private
{
public:
    QPointer<ConfigWidget> widget;
    std::unique_ptr<mu::Parser> parser;
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

    // Load settings
    d->locale.setNumberOptions(
                (settings().value(CFG_SEPS, CFG_SEPS_DEF).toBool())
                ? d->locale.numberOptions() & ~QLocale::OmitGroupSeparator
                : d->locale.numberOptions() | QLocale::OmitGroupSeparator );

    QString iconPath = XDG::IconLookup::iconPath("calc");
    d->iconPath = iconPath.isNull() ? ":calc" : iconPath;

    d->parser.reset(new mu::Parser);
    d->parser->SetDecSep(d->locale.decimalPoint().toLatin1());
    d->parser->SetThousandsSep(d->locale.groupSeparator().toLatin1());
}



/** ***************************************************************************/
Calculator::Extension::~Extension() {

}



/** ***************************************************************************/
QWidget *Calculator::Extension::widget(QWidget *parent) {
    if (d->widget.isNull()){
        d->widget = new ConfigWidget(parent);

        d->widget->ui.checkBox_groupsep->setChecked(!(d->locale.numberOptions() & QLocale::OmitGroupSeparator));
        connect(d->widget->ui.checkBox_groupsep, &QCheckBox::toggled, [this](bool checked){
            settings().setValue(CFG_SEPS, checked);
            d->locale.setNumberOptions( (checked) ? d->locale.numberOptions() & ~QLocale::OmitGroupSeparator
                                                  : d->locale.numberOptions() | QLocale::OmitGroupSeparator );
        });
    }
    return d->widget;
}



/** ***************************************************************************/
void Calculator::Extension::handleQuery(Core::Query * query) const {

    d->parser->SetExpr(query->string().toStdString());
    QString result;

    // http://beltoforion.de/article.php?a=muparser&p=errorhandling
    try {
        result = d->locale.toString(d->parser->Eval(), 'G', 16);
    } catch (mu::Parser::exception_type &) {
        return;
    }

    auto item = make_shared<StandardItem>("muparser");
    item->setText(result);
    item->setSubtext(QString("Result of '%1'").arg(query->string()));
    item->setIconPath(d->iconPath);
    item->addAction(make_shared<ClipAction>("Copy result to clipboard", result));
    item->addAction(make_shared<ClipAction>("Copy equation to clipboard",
                                            QString("%1 = %2").arg(query->string(), result)));
    query->addMatch(move(item), UINT_MAX);
}
