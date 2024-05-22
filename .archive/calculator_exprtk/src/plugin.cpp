// Copyright (c) 2023-2024 Manuel Schneider

#include "albert/albert.h"
#include "albert/logging.h"
#include "albert/util/standarditem.h"
#include "plugin.h"
#include <gmp.h>
#include <mpfr.h>
#include "exprtk.hpp"
#include <QLabel>
ALBERT_LOGGING_CATEGORY("exprtk")
using namespace albert;
using namespace exprtk;
using namespace std;
using T = double;


class Plugin::Private
{
public:
    symbol_table<T> symbol_table;
    expression<T>   expression;
    parser<T>       parser;

#if defined Q_OS_MACOS
    const QStringList icon_urls = {"qfip:/System/Applications/Calculator.app"};
#elif defined(Q_OS_LINUX) || defined(Q_OS_FREEBSD)
    const QStringList icon_urls = {"xdg:calc"};
#endif
};


Plugin::Plugin() : d(new Private)
{
    d->symbol_table.add_constants();
    d->expression.register_symbol_table(d->symbol_table);
}

Plugin::~Plugin() = default;

QString Plugin::defaultTrigger() const
{ return "="; }

vector<RankItem> Plugin::handleGlobalQuery(const Query *query) const
{
    vector<RankItem> results;
    if (d->parser.compile(query->string().toStdString(), d->expression))
    {
        T result = d->expression.value();
        auto query_string = query->string();
        auto result_str = QString::number(result, 'g', 16);

        results.emplace_back(
            StandardItem::make(
                QStringLiteral("qalc-res"),
                result_str,
                tr("Result of %1").arg(query_string),
                d->icon_urls,
                {
                    {
                        "cpr", tr("Copy result to clipboard"),
                        [=]{ setClipboardText(result_str); }
                    },
                    {
                        "cpe", tr("Copy equation to clipboard"),
                        [=]{ setClipboardText(QString("%1 = %2").arg(query_string, result_str)); }
                    }
                }
            ),
            1.0f
        );
    }
    return results;
}


QWidget *Plugin::buildConfigWidget()
{
    auto l = new QLabel;
    l->setText(tr(
        "This plugin is based on the %1 library and uses double precision "
        "floats which have a well known <a href=%2>accuracy problem</a>.")
               .arg("<a href=\"https://www.partow.net/programming/exprtk/\">exprtk</a>",
                    "\"https://en.wikipedia.org/wiki/Floating-point_arithmetic#Accuracy_problems\"")
    );
    l->setAlignment(Qt::AlignTop);
    l->setOpenExternalLinks(true);
    l->setTextFormat(Qt::RichText);
    l->setWordWrap(true);
    return l;
}

