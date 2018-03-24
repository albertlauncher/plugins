/*
 *   Copyright (C) 2014-2017 Manuel Schneider
 *   Copyright (C) 2018 Jakob Riepler
 *   Copyright (C) 2007 Barış Metin <baris@pardus.org.tr>
 *   Copyright (C) 2006 David Faure <faure@kde.org>
 *   Copyright (C) 2007 Richard Moore <rich@kde.org>
 *   Copyright (C) 2010 Matteo Agostinelli <agostinelli@gmail.com>
 *   
 *   https://github.com/KDE/plasma-workspace/blob/master/runners/calculator/calculatorrunner.cpp
 */

#include <QClipboard>
#include <QDebug>
#include <QLocale>
#include <QPointer>
#include <QSettings>
#include <vector>
#include "configwidget.h"
#include "extension.h"
#include "core/query.h"
#include "util/standarditem.h"
#include "util/standardactions.h"
#include "xdg/iconlookup.h"

#include "qalculate_engine.h"

using namespace std;
using namespace Core;

class Qalculate::Private
{
public:
    QPointer<ConfigWidget> widget;
    QalculateEngine* engine;
    QLocale locale;
    QString iconPath;
};



/** ***************************************************************************/
Qalculate::Extension::Extension()
    : Core::Extension("org.albert.extension.qalculate"),
      Core::QueryHandler(Core::Plugin::id()),
      d(new Private){

    registerQueryHandler(this);

    // FIXME Qt6 Workaround for https://bugreports.qt.io/browse/QTBUG-58504
    d->locale = QLocale(QLocale::system().name());

    QString iconPath = XDG::IconLookup::iconPath("calc");
    d->iconPath = iconPath.isNull() ? ":calc" : iconPath;

    d->engine = new QalculateEngine;
}



/** ***************************************************************************/
Qalculate::Extension::~Extension() {
    delete d->engine;
}



/** ***************************************************************************/
QWidget *Qalculate::Extension::widget(QWidget *parent) {
    if (d->widget.isNull()){
        d->widget = new ConfigWidget(parent);
    }
    return d->widget;
}



/** ***************************************************************************/
void Qalculate::Extension::handleQuery(Core::Query * query) const {

    QString cmd = query->string();

    cmd = cmd.trimmed().remove(QLatin1Char(' '));

    if (cmd.length() < 3) {
        return;
    }

    if (cmd.toLower() == QLatin1String("universe") || cmd.toLower() == QLatin1String("life") || cmd.toLower() == QLatin1String("everything")) {
        auto life = make_shared<StandardItem>("qalculate");
        life->setIconPath(d->iconPath);
    life->setText(QString("42"));
        life->setSubtext(QString("The answer to life, the universe and everything."));
    life->setCompletion(life->text());
        query->addMatch(move(life), 42);
        return;
    }

    bool toHex = cmd.startsWith(QLatin1String("hex="));
    bool startsWithEquals = !toHex && cmd[0] == QLatin1Char('=');


    if (toHex || startsWithEquals) {
        cmd.remove(0, cmd.indexOf(QLatin1Char('=')) + 1);
    } else if (cmd.endsWith(QLatin1Char('='))) {
        cmd.chop(1);
    } else {
        bool foundDigit = false;
        for (int i = 0; i < cmd.length(); ++i) {
            QChar c = cmd.at(i);
            if (c.isLetter()) {
                // not just numbers and symbols, so we return
                return;
            }
            if (c.isDigit()) {
                foundDigit = true;
            }
        }
        if (!foundDigit) {
            return;
        }
    }

    if (cmd.isEmpty()) {
        return;
    }

    bool isApproximate = false;

    QString result;
    try {
        result = d->engine->evaluate(cmd, &isApproximate);
    } catch(std::exception& e) {
        qDebug() << "qalculate error: " << e.what();
    }

    if (!result.isEmpty() && result != cmd) {
        if (toHex) {
            result = QStringLiteral("0x") + QString::number(result.toInt(), 16).toUpper();
        }

        auto item = make_shared<StandardItem>("qalculate");
        
    item->setIconPath(d->iconPath);
    item->setText(result);
    if (!isApproximate) {
        item->setSubtext(QString("Result of '%1'").arg(cmd));
    } else {
        item->setSubtext(QString("Approximate result of '%1'").arg(cmd));
    }
    item->setCompletion(item->text());
        item->addAction(make_shared<ClipAction>("Copy result to clipboard", result));
    item->addAction(make_shared<ClipAction>("Copy equation to clipboard", QString("%1 = %2").arg(cmd, item->text())));
    query->addMatch(move(item), UINT_MAX);
    }
}
