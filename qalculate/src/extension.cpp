// Copyright (C) 2014-2018 Manuel Schneider
// Copyright (C) 2018 Jakob Riepler

#include <QClipboard>
#include <QDebug>
#include <QPointer>
#include <QSettings>
#include <vector>
#include "configwidget.h"
#include "extension.h"
#include "core/query.h"
#include "util/standarditem.h"
#include "util/standardactions.h"
#include "xdg/iconlookup.h"

#include <libqalculate/Calculator.h>

using namespace std;
using namespace Core;

namespace {

const char* CFG_ANGLEUNIT = "angle_unit";
const uint  DEF_ANGLEUNIT = 1;
const char* CFG_PRECISION = "precision";
const uint  DEF_PRECISION = 16;

}

class Qalculate::Private
{
public:
    QPointer<ConfigWidget> widget;
    QString iconPath;
    unique_ptr<Calculator> calculator;
    EvaluationOptions eo;
    PrintOptions po;
};



/** ***************************************************************************/
Qalculate::Extension::Extension()
    : Core::Extension("org.albert.extension.qalculate"),
      Core::QueryHandler(Core::Plugin::id()),
      d(new Private){

    registerQueryHandler(this);

    QString iconPath = XDG::IconLookup::iconPath("calc");
    d->iconPath = iconPath.isNull() ? ":calc" : iconPath;

    d->calculator.reset(new Calculator());
    d->calculator->loadGlobalDefinitions();
    d->calculator->loadLocalDefinitions();
    d->calculator->loadGlobalCurrencies();
    d->calculator->loadExchangeRates();

    // Set evaluation options
    d->eo.parse_options.angle_unit = ANGLE_UNIT_RADIANS;
    d->eo.structuring = STRUCTURING_SIMPLIFY;
    d->eo.auto_post_conversion = POST_CONVERSION_OPTIMAL;
    d->eo.keep_zero_units = false;
    d->eo.parse_options.limit_implicit_multiplication = true;

    // Set print options
    d->po.lower_case_e = true;
    d->po.preserve_precision = true;
    d->po.use_unicode_signs = true;
    d->po.indicate_infinite_series = true;

    // Load settings
    d->eo.parse_options.angle_unit = static_cast<AngleUnit>(settings().value(CFG_ANGLEUNIT, DEF_ANGLEUNIT).toInt());
    d->calculator->setPrecision(settings().value(CFG_PRECISION, DEF_PRECISION).toInt());
}



/** ***************************************************************************/
Qalculate::Extension::~Extension() {

}



/** ***************************************************************************/
QWidget *Qalculate::Extension::widget(QWidget *parent) {
    if (d->widget.isNull()){
        d->widget = new ConfigWidget(parent);

        // Angle unit
        d->widget->ui.angleUnitComboBox->setCurrentIndex(d->eo.parse_options.angle_unit);
        connect(d->widget->ui.angleUnitComboBox,
                static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
                this, [this](int index){
            settings().setValue(CFG_ANGLEUNIT, index);
            d->eo.parse_options.angle_unit = static_cast<AngleUnit>(index);
        });

        // Precision
        d->widget->ui.precisionSpinBox->setValue(d->calculator->getPrecision());
        connect(d->widget->ui.precisionSpinBox,
                static_cast<void(QSpinBox::*)(int)>(&QSpinBox::valueChanged),
                this, [this](int value){
            settings().setValue(CFG_PRECISION, value);
            d->calculator->setPrecision(value);
        });
    }
    return d->widget;
}



/** ***************************************************************************/
void Qalculate::Extension::handleQuery(Core::Query * query) const {

    if (query->string().trimmed().isEmpty())
        return;

    d->eo.parse_options.functions_enabled = query->isTriggered();
    d->eo.parse_options.units_enabled = query->isTriggered();
    d->eo.parse_options.unknowns_enabled = query->isTriggered();

    QString result, error, cmd = query->string().trimmed();
    MathStructure mathStructure;
    try {
        string expr = d->calculator->unlocalizeExpression(query->string().toStdString(), d->eo.parse_options);
        mathStructure = d->calculator->calculate(expr, d->eo);
        while (d->calculator->message()) {
            error.append(d->calculator->message()->c_message());
            d->calculator->nextMessage();
        }
    } catch(std::exception& e) {
        qDebug() << "qalculate error: " << e.what();
    }

    if (error.isNull()){
        result = QString::fromStdString(mathStructure.print(d->po));
        auto item = make_shared<StandardItem>(Plugin::id());
        item->setIconPath(d->iconPath);
        item->setText(result);
        if ( mathStructure.isApproximate() )
            item->setSubtext(QString("Approximate result of '%1'").arg(cmd));
        else
            item->setSubtext(QString("Result of '%1'").arg(cmd));
        item->setCompletion(QString("=%1").arg(result));
        item->addAction(make_shared<ClipAction>("Copy result to clipboard", result));
        item->addAction(make_shared<ClipAction>("Copy equation to clipboard", QString("%1 = %2").arg(cmd, item->text())));
        query->addMatch(move(item), UINT_MAX);
    }
    else {
        if (query->isTriggered()){
            result = QString::fromStdString(mathStructure.print(d->po));
            auto item = make_shared<StandardItem>(Plugin::id());
            item->setIconPath(d->iconPath);
            item->setText("Evaluation error.");
            item->setSubtext(error);
            item->setCompletion(query->string());
            query->addMatch(move(item), UINT_MAX);
        }
    }
}
