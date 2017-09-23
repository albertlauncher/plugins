// albert - a simple application launcher for linux
// Copyright (C) 2014-2017 Manuel Schneider
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

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
#include "util/standardaction.h"
#include "xdg/iconlookup.h"
using std::vector;
using std::shared_ptr;
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

    // Build actions
    vector<shared_ptr<Action>> actions;
    shared_ptr<StandardAction> action = std::make_shared<StandardAction>();
    action->setText(QString("Copy '%1' to clipboard").arg(result));
    action->setAction([=](){ QApplication::clipboard()->setText(result); });
    actions.push_back(action);

    // Make searchterm a lvalue that can be captured by the lambda
    QString text = query->string();
    action = std::make_shared<StandardAction>();
    action->setText(QString("Copy '%1' to clipboard").arg(text));
    action->setAction([=](){ QApplication::clipboard()->setText(text); });
    actions.push_back(action);

    text = QString("%1 = %2").arg(query->string(), result);
    action = std::make_shared<StandardAction>();
    action->setText(QString("Copy '%1' to clipboard").arg(text));
    action->setAction([=](){ QApplication::clipboard()->setText(text); });
    actions.push_back(action);

    shared_ptr<StandardItem> calcItem = std::make_shared<StandardItem>("muparser-eval");
    calcItem->setText(result);
    calcItem->setSubtext(QString("Result of '%1'").arg(query->string()));
    calcItem->setIconPath(d->iconPath);
    calcItem->setActions(std::move(actions));

    query->addMatch(std::move(calcItem), UINT_MAX);
}
