// Copyright (c) 2023-2024 Manuel Schneider

#include "albert/albert.h"
#include "albert/extension/queryhandler/standarditem.h"
#include "albert/logging.h"
#include "plugin.h"
#include "ui_configwidget.h"
#include <QSettings>
#include <QThread>
ALBERT_LOGGING_CATEGORY("qalculate")
using namespace albert;
using namespace std;

namespace {
const char* URL_MANUAL = "https://qalculate.github.io/manual/index.html";
const char* CFG_ANGLEUNIT = "angle_unit";
const uint  DEF_ANGLEUNIT = (int)ANGLE_UNIT_RADIANS;
const char* CFG_PARSINGMODE = "parsing_mode";
const uint  DEF_PARSINGMODE = (int)PARSING_MODE_CONVENTIONAL;
const char* CFG_PRECISION = "precision";
const uint  DEF_PRECISION = 16;
}

const QStringList Plugin::icon_urls = {"xdg:calc", ":qalculate"};

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

QString Plugin::defaultTrigger() const
{ return "="; }

QString Plugin::synopsis() const
{
    static const auto tr_me = tr("<math expression>");
    return tr_me;
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
        lock_guard locker(qalculate_mutex);
        eo.parse_options.angle_unit = static_cast<AngleUnit>(index);
    });

    // Parsing mode
    ui.parsingModeComboBox->setCurrentIndex(eo.parse_options.parsing_mode);
    connect(ui.parsingModeComboBox,
            static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
            this, [this](int index){
        settings()->setValue(CFG_PARSINGMODE, index);
        lock_guard locker(qalculate_mutex);
        eo.parse_options.parsing_mode = static_cast<ParsingMode>(index);
    });

    // Precision
    ui.precisionSpinBox->setValue(qalc->getPrecision());
    connect(ui.precisionSpinBox,
            static_cast<void(QSpinBox::*)(int)>(&QSpinBox::valueChanged),
            this, [this](int value){
        settings()->setValue(CFG_PRECISION, value);
        lock_guard locker(qalculate_mutex);
        qalc->setPrecision(value);
    });

    return widget;
}

shared_ptr<Item> Plugin::buildItem(const QString &query, const MathStructure &mstruct) const
{
    static const auto tr_tr = tr("Copy result to clipboard");
    static const auto tr_te = tr("Copy equation to clipboard");
    static const auto tr_e = tr("Result of %1");
    static const auto tr_a = tr("Approximate result of %1");
    auto result = QString::fromStdString(mstruct.print(po));

    return StandardItem::make(
        "qalc-res",
        result,
        mstruct.isApproximate() ? tr_a.arg(query) : tr_e.arg(query),
        QString("%1%2").arg(trigger(), result),
        icon_urls,
        {
            {"cpr", tr_tr, [=](){ setClipboardText(result); }},
            {"cpe", tr_te, [=](){ setClipboardText(QString("%1 = %2").arg(query, result)); }}
        }
    );
}

vector<RankItem> Plugin::handleGlobalQuery(const GlobalQuery *query) const
{
    vector<RankItem> results;

    auto trimmed = query->string().trimmed();
    if (trimmed.isEmpty())
        return results;

    lock_guard locker(qalculate_mutex);

    auto expression = qalc->unlocalizeExpression(query->string().toStdString(), eo.parse_options);

    qalc->startControl();
    MathStructure mstruct;
    qalc->calculate(&mstruct, expression, 0, eo);
    for (; qalc->busy(); QThread::msleep(10))
        if (!query->isValid())
            qalc->abort();
    qalc->stopControl();

    if (!query->isValid())
        return results;

    if (qalc->message()){
        for (auto msg = qalc->message(); msg; msg = qalc->nextMessage())
            DEBG << QString::fromUtf8(qalc->message()->c_message());
        return results;
    }

    mstruct.format(po);

    results.emplace_back(buildItem(trimmed, mstruct), 1.0f);

    return results;
}

void Plugin::handleTriggerQuery(TriggerQuery *query) const
{
    auto trimmed = query->string().trimmed();
    if (trimmed.isEmpty())
        return;

    lock_guard locker(qalculate_mutex);

    auto eo_ = eo;
    eo_.parse_options.functions_enabled = true;
    eo_.parse_options.units_enabled = true;
    eo_.parse_options.unknowns_enabled = true;

    auto expression = qalc->unlocalizeExpression(query->string().toStdString(), eo_.parse_options);

    qalc->startControl();
    MathStructure mstruct;
    qalc->calculate(&mstruct, expression, 0, eo_);
    for (; qalc->busy(); QThread::msleep(10))
        if (!query->isValid())
            qalc->abort();
    qalc->stopControl();

    if (!query->isValid())
        return;

    QStringList errors;
    for (auto msg = qalc->message(); msg; msg = qalc->nextMessage())
        errors << QString::fromUtf8(qalc->message()->c_message());

    if (errors.empty())
    {
        mstruct.format(po);
        query->add(buildItem(trimmed, mstruct));
    }
    else
    {
        static const auto tr_e = tr("Evaluation error.");
        static const auto tr_d = tr("Visit documentation");
        query->add(
            StandardItem::make(
                "qalc-err",
                tr_e,
                errors.join(" "),
                icon_urls,
                {{"manual", tr_d, [=](){ openUrl(URL_MANUAL); }}}
            )
        );
    }
}
