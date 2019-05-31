// Copyright (C) 2014-2018 Manuel Schneider

#include <QClipboard>
#include <QDebug>
#include <QLocale>
#include <QPointer>
#include <QSettings>
#include <vector>
#include "configwidget.h"
#include "extension.h"
#include "muParser.h"
#include "albert/query.h"
#include "albert/util/standarditem.h"
#include "albert/util/standardactions.h"
#include "xdg/iconlookup.h"
using namespace std;
using namespace Core;

Q_LOGGING_CATEGORY(qlc_calculator, "calculator")


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

    QString iconPath = XDG::IconLookup::iconPath("calc");
    d->iconPath = iconPath.isNull() ? ":calc" : iconPath;

    d->parser.reset(new mu::Parser);
    d->parser->SetDecSep(d->locale.decimalPoint().toLatin1());
    d->parser->SetThousandsSep(d->locale.groupSeparator().toLatin1());
    d->parser->SetArgSep(';');
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

    try {
        d->parser->SetExpr(query->string().toStdString());
    } catch (mu::Parser::exception_type &exception) {
        qCWarning(qlc_calculator).noquote() << "Muparser SetExpr exception: " << exception.GetMsg().c_str();
        return;
    }
    double result;

    // http://beltoforion.de/article.php?a=muparser&p=errorhandling
    try {
        result = d->parser->Eval();
    } catch (mu::Parser::exception_type &) {
        // Expected exception in case of invalid input
        // qDebug() << "Muparser Eval exception: " << exception.GetMsg().c_str();
        return;
    }

    auto item = make_shared<StandardItem>("muparser");
    item->setIconPath(d->iconPath);
    d->locale.setNumberOptions(settings().value(CFG_SEPS, CFG_SEPS_DEF).toBool()
                               ? d->locale.numberOptions() & ~QLocale::OmitGroupSeparator
                               : d->locale.numberOptions() | QLocale::OmitGroupSeparator );
    item->setText(d->locale.toString(result, 'G', 16));
    item->setSubtext(QString("Result of '%1'").arg(query->string()));
    item->setCompletion(item->text());
    d->locale.setNumberOptions(d->locale.numberOptions() | QLocale::OmitGroupSeparator);
    item->addAction(make_shared<ClipAction>("Copy result to clipboard",
                                            d->locale.toString(result, 'G', 16)));
    item->addAction(make_shared<ClipAction>("Copy equation to clipboard",
                                            QString("%1 = %2").arg(query->string(), item->text())));
    query->addMatch(move(item), UINT_MAX);
}
