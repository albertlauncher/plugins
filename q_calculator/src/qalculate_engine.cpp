/*
*   Copyright 2010 Matteo Agostinelli <agostinelli@gmail.com>
*
*   Original source: https://github.com/KDE/plasma-workspace/blob/master/runners/calculator/qalculate_engine.cpp
*
*   This program is free software; you can redistribute it and/or modify
*   it under the terms of the GNU Library General Public License as
*   published by the Free Software Foundation; either version 2 or
*   (at your option) any later version.
*
*   This program is distributed in the hope that it will be useful,
*   but WITHOUT ANY WARRANTY; without even the implied warranty of
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*   GNU General Public License for more details
*
*   You should have received a copy of the GNU Library General Public
*   License along with this program; if not, write to the
*   Free Software Foundation, Inc.,
*   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

#include "qalculate_engine.h"

#include <libqalculate/Calculator.h>
#include <libqalculate/ExpressionItem.h>
#include <libqalculate/Unit.h>
#include <libqalculate/Prefix.h>
#include <libqalculate/Variable.h>
#include <libqalculate/Function.h>

#include <QFile>
#include <QApplication>
#include <QClipboard>
#include <QDebug>
#include <QNetworkRequest>
#include <QNetworkAccessManager>

QAtomicInt QalculateEngine::s_counter;

QalculateEngine::QalculateEngine(QObject* parent):
    QObject(parent)
{
    m_lastResult = "";
    s_counter.ref();
    if (!CALCULATOR) {
        new Calculator();
        CALCULATOR->terminateThreads();
        CALCULATOR->loadGlobalDefinitions();
        CALCULATOR->loadLocalDefinitions();
        CALCULATOR->loadGlobalCurrencies();
        CALCULATOR->loadExchangeRates();
    }
}

QalculateEngine::~QalculateEngine()
{
    if (s_counter.deref()) {
        delete CALCULATOR;
        CALCULATOR = nullptr;
    }
}

void QalculateEngine::updateExchangeRates()
{
    QUrl source = QUrl("http://www.ecb.int/stats/eurofxref/eurofxref-daily.xml");

    QNetworkAccessManager getJob;
    QNetworkRequest request(source);
    connect( getJob.get(request), SIGNAL(finished(QNetworkReply*)), this, SLOT(updateResult(QNetworkReply*)) );
}

void QalculateEngine::updateResult(QNetworkReply* reply)
{
    QUrl dest = QUrl::fromLocalFile(QFile::decodeName(CALCULATOR->getExchangeRatesFileName().c_str()));

    if(reply->error() == QNetworkReply::NoError) {
        int httpstatuscode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toUInt();
	if (httpstatuscode == 200) {
            QFile file(dest.fileName());
            if (!file.open(QIODevice::WriteOnly)) {
                qDebug() << "Could not open " << qPrintable(dest.fileName()) << " for writing: " << qPrintable(file.errorString()) << "\n";
                return;
            }

            file.write(reply->readAll());
            file.close();

            // the exchange rates have been successfully updated, now load them
            CALCULATOR->loadExchangeRates();
	} else {
             qDebug() << "The exchange rates could not be updated. The following error has been reported: HTTP " << httpstatuscode << "\n";
	}
    } else {
        qDebug() << "The exchange rates could not be updated. The following error has been reported: " << reply->errorString() << "\n";
    }
}

QString QalculateEngine::evaluate(const QString& expression, bool *isApproximate)
{
    if (expression.isEmpty()) {
        return "";
    }

    QString input = expression;
    QByteArray ba = input.replace(QChar(0xA3), "GBP").replace(QChar(0xA5), "JPY").replace('$', "USD").replace(QChar(0x20AC), "EUR").toLatin1();
    const char *ctext = ba.data();

    CALCULATOR->terminateThreads();
    EvaluationOptions eo;

    eo.auto_post_conversion = POST_CONVERSION_BEST;
    eo.keep_zero_units = false;

    eo.parse_options.angle_unit = ANGLE_UNIT_RADIANS;
    eo.structuring = STRUCTURING_SIMPLIFY;

    // suggested in https://github.com/Qalculate/libqalculate/issues/16
    // to avoid memory overflow for seemingly innocent calculations (Bug 277011)
    eo.approximation = APPROXIMATION_APPROXIMATE;

    CALCULATOR->setPrecision(16);
    MathStructure result = CALCULATOR->calculate(ctext, eo);

    PrintOptions po;
    po.number_fraction_format = FRACTION_DECIMAL;
    po.indicate_infinite_series = false;
    po.use_all_prefixes = false;
    po.use_denominator_prefix = true;
    po.negative_exponents = false;
    po.lower_case_e = true;
    po.base_display = BASE_DISPLAY_NORMAL;

    result.format(po);

    m_lastResult = result.print(po).c_str();

    if (isApproximate) {
        *isApproximate = result.isApproximate();
    }

    return m_lastResult;
}

void QalculateEngine::copyToClipboard(bool flag)
{
    Q_UNUSED(flag);

    QApplication::clipboard()->setText(m_lastResult);
}
