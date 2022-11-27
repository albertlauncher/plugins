// Copyright (c) 2022 Manuel Schneider

#pragma once
#include "albert.h"
#include "muParser.h"
#include "muParserInt.h"
#include <memory>
#include <QLocale>

class Plugin:
        public albert::ExtensionPlugin,
        public albert::GlobalQueryHandler,
        public albert::ConfigWidgetProvider
{
    Q_OBJECT ALBERT_PLUGIN

public:
    Plugin();

    std::vector<albert::RankItem> rankItems(const QString &string, const bool& isValid) const override;
    QWidget* buildConfigWidget() override;

    void setGroupSeparatorEnabled(bool enabled);
    void setIParserEnabled(bool enabled);

private:
    std::unique_ptr<mu::Parser> parser;
    std::unique_ptr<mu::ParserInt> iparser;
    QLocale locale;
};
