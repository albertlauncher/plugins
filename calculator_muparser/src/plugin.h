// Copyright (c) 2022 Manuel Schneider

#pragma once
#include "albert/extension/queryhandler/globalqueryhandler.h"
#include "albert/plugin.h"
#include "muParser.h"
#include "muParserInt.h"
#include <QLocale>
#include <memory>


class Plugin : public albert::plugin::ExtensionPlugin<albert::GlobalQueryHandler>
{
    Q_OBJECT ALBERT_PLUGIN

public:
    Plugin();

    QString defaultTrigger() const override;
    QString synopsis() const override;
    std::vector<albert::RankItem> handleGlobalQuery(const GlobalQuery*) const override;
    QWidget* buildConfigWidget() override;

    void setGroupSeparatorEnabled(bool enabled);
    void setIParserEnabled(bool enabled);

private:
    std::unique_ptr<mu::Parser> parser;
    std::unique_ptr<mu::ParserInt> iparser;
    QLocale locale;
};
