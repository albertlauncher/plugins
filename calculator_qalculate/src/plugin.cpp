// Copyright (c) 2023 Manuel Schneider

#include "plugin.h"
#include "ui_configwidget.h"
ALBERT_LOGGING
using namespace std;
using namespace albert;

namespace {
const char* URL_MANUAL = "https://qalculate.github.io/manual/index.html";
const char* CFG_ANGLEUNIT = "angle_unit";
const uint  DEF_ANGLEUNIT = (int)ANGLE_UNIT_RADIANS;
const char* CFG_PARSINGMODE = "parsing_mode";
const uint  DEF_PARSINGMODE = (int)PARSING_MODE_CONVENTIONAL;
const char* CFG_PRECISION = "precision";
const uint  DEF_PRECISION = 16;
}

Plugin::Plugin()
{
    auto s = settings();

    // init calculator
    qalc.reset(new Calculator());
    qalc->loadExchangeRates();
    qalc->loadGlobalCurrencies();
    qalc->loadGlobalDefinitions();
    qalc->loadLocalDefinitions();
    qalc->setPrecision(s->value(CFG_PRECISION, DEF_PRECISION).toInt());

    // evaluation options
    eo.auto_post_conversion = POST_CONVERSION_BEST;
    eo.structuring = STRUCTURING_SIMPLIFY;

    // parse options
    eo.parse_options.angle_unit = static_cast<AngleUnit>(s->value(CFG_ANGLEUNIT, DEF_ANGLEUNIT).toInt());
    eo.parse_options.functions_enabled = false;
    eo.parse_options.limit_implicit_multiplication = true;
    eo.parse_options.parsing_mode = static_cast<ParsingMode>(s->value(CFG_PARSINGMODE, DEF_PARSINGMODE).toInt());
    eo.parse_options.units_enabled = false;
    eo.parse_options.unknowns_enabled = false;

    // print options
    po.indicate_infinite_series = true;
    po.interval_display = INTERVAL_DISPLAY_SIGNIFICANT_DIGITS;
    po.lower_case_e = true;
    //po.preserve_precision = true;  // https://github.com/albertlauncher/plugins/issues/92
    po.use_unicode_signs = true;
    //po.abbreviate_names = true;
}

QWidget *Plugin::buildConfigWidget()
{
    auto *widget = new QWidget;
    Ui::ConfigWidget ui;
    ui.setupUi(widget);

    // Angle unit
    ui.angleUnitComboBox->setCurrentIndex(eo.parse_options.angle_unit);
    connect(ui.angleUnitComboBox,
            static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
            this, [this](int index){
        settings()->setValue(CFG_ANGLEUNIT, index);
        eo.parse_options.angle_unit = static_cast<AngleUnit>(index);
    });

    // Parsing mode
    ui.parsingModeComboBox->setCurrentIndex(eo.parse_options.parsing_mode);
    connect(ui.parsingModeComboBox,
            static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
            this, [this](int index){
        settings()->setValue(CFG_PARSINGMODE, index);
        eo.parse_options.parsing_mode = static_cast<ParsingMode>(index);
    });

    // Precision
    ui.precisionSpinBox->setValue(qalc->getPrecision());
    connect(ui.precisionSpinBox,
            static_cast<void(QSpinBox::*)(int)>(&QSpinBox::valueChanged),
            this, [this](int value){
        settings()->setValue(CFG_PRECISION, value);
        qalc->setPrecision(value);
    });

    return widget;
}

vector<RankItem> Plugin::handleGlobalQuery(const GlobalQuery &query) const
{
    vector<RankItem> results;

    auto trimmed = query.string().trimmed();
    if (trimmed.isEmpty())
        return results;

    auto expression = qalc->unlocalizeExpression(query.string().toStdString(), eo.parse_options);
    auto mstruct = qalc->calculate(expression, eo);
    if (qalc->message()){
        qalc->clearMessages();
        return results;
    }

    mstruct.format(po);
    auto result = QString::fromStdString(mstruct.print(po));

    results.emplace_back(
        StandardItem::make(
            "qalc-res",
            result,
            QString("%1esult of %2").arg(mstruct.isApproximate()?"Approximate r":"R", trimmed),
            result,   // TODO if handler finally knows its trigger change this to fire a triggered query
            {"xdg:calc", ":qalculate"},
            {
                {
                    "cpr", "Copy result to clipboard",
                    [=](){ setClipboardText(result); }
                }, {
                    "cpe", "Copy equation to clipboard",
                    [=](){ setClipboardText(QString("%1 = %2").arg(trimmed, result)); }
                }
            }
        ),
        RankItem::MAX_SCORE);
    return results;
}



void Plugin::handleTriggerQuery(TriggerQuery &query) const
{
    auto trimmed = query.string().trimmed();
    if (trimmed.isEmpty())
        return;

    auto eo_ = eo;
    eo_.parse_options.functions_enabled = true;
    eo_.parse_options.units_enabled = true;
    eo_.parse_options.unknowns_enabled = true;

    auto expression = qalc->unlocalizeExpression(query.string().toStdString(), eo_.parse_options);
    auto mstruct = qalc->calculate(expression, eo_);
    QStringList errors;
    for (auto msg = qalc->message(); msg; msg = qalc->nextMessage())
        errors << QString::fromUtf8(qalc->message()->c_message());
    mstruct.format(po);

    if (errors.empty()){
        auto result = QString::fromStdString(mstruct.print(po));
        query.add(
            StandardItem::make(
                "qalc-res",
                result,
                QString("%1esult of %2").arg(mstruct.isApproximate()?"Approximate r":"R", trimmed),
                QString("%1%2").arg(query.trigger(), result),
                {"xdg:calc", ":qalculate"},
                {
                    {
                        "cpr", "Copy result to clipboard",
                        [=](){ setClipboardText(result); }
                    }, {
                        "cpe", "Copy equation to clipboard",
                        [=](){ setClipboardText(QString("%1 = %2").arg(trimmed, result)); }
                    }
                }
            )
        );
    }
    else
        query.add(
            StandardItem::make(
                "qalc-err",
                "Evaluation error.",
                errors.join(" "),
                {"xdg:calc", ":qalculate"},
                {{"manual", "Visit documentation", [=](){ openUrl(URL_MANUAL); }}}
            )
        );
}
