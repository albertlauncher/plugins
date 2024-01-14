// Copyright (C) 2023-2024 Manuel Schneider

#pragma once
#include "albert/extension/queryhandler/globalqueryhandler.h"
#include "albert/plugin.h"
#include <QObject>
#include <libqalculate/Calculator.h>
#include <memory>

class Plugin : public albert::plugin::ExtensionPlugin,
               public albert::GlobalQueryHandler
{
    Q_OBJECT ALBERT_PLUGIN
public:
    Plugin();

    QString defaultTrigger() const override;
    QString synopsis() const override;
    std::vector<albert::RankItem> handleGlobalQuery(const GlobalQuery*) const override;
    void handleTriggerQuery(TriggerQuery*) const override;
    QWidget* buildConfigWidget() override;

private:
    std::shared_ptr<albert::Item> buildItem(const QString &query, const MathStructure &mstruct) const;

    QString iconPath;
    std::unique_ptr<Calculator> qalc;
    EvaluationOptions eo;
    PrintOptions po;
    mutable std::mutex qalculate_mutex;
    static const QStringList icon_urls;
};
