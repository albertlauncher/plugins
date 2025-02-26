// Copyright (C) 2023-2024 Manuel Schneider

#pragma once
#include <albert/globalqueryhandler.h>
#include <albert/extensionplugin.h>
#include <QObject>
#include <libqalculate/Calculator.h>
#include <memory>

class Plugin : public albert::ExtensionPlugin,
               public albert::GlobalQueryHandler
{
    ALBERT_PLUGIN

public:

    Plugin();

    QString defaultTrigger() const override;
    QString synopsis(const QString &) const override;
    void handleTriggerQuery(albert::Query &) override;
    std::vector<albert::RankItem> handleGlobalQuery(const albert::Query &) override;
    QWidget* buildConfigWidget() override;

private:

    std::variant<QStringList, MathStructure>
    runQalculateLocked(const albert::Query &, const EvaluationOptions &eo) ;

    std::shared_ptr<albert::Item> buildItem(const QString &query, const MathStructure &mstruct) const;

    QString iconPath;
    std::unique_ptr<Calculator> qalc;
    EvaluationOptions eo;
    PrintOptions po;
    std::mutex qalculate_mutex;
    static const QStringList icon_urls;

};
