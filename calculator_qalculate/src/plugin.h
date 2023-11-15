// Copyright (C) 2023 Manuel Schneider

#pragma once
#include "albert/extension/queryhandler/globalqueryhandler.h"
#include "albert/plugin.h"
#include <QObject>
#include <libqalculate/Calculator.h>
#include <memory>

class Plugin : public albert::plugin::ExtensionPlugin<albert::GlobalQueryHandler>
{
    Q_OBJECT ALBERT_PLUGIN
public:
    Plugin();

    QString defaultTrigger() const override { return "="; }
    QString synopsis() const override { return "<math expression>"; }
    std::vector<albert::RankItem> handleGlobalQuery(const GlobalQuery*) const override;
    void handleTriggerQuery(TriggerQuery*) const override;
    QWidget* buildConfigWidget() override;

private:
    QString iconPath;
    std::unique_ptr<Calculator> qalc;
    EvaluationOptions eo;
    PrintOptions po;
};
