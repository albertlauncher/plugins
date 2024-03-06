// Copyright (c) 2017-2024 Manuel Schneider

#pragma once
#include "albert/query/globalqueryhandler.h"
#include "albert/util/extensionplugin.h"

class Plugin : public albert::ExtensionPlugin,
               public albert::GlobalQueryHandler
{
    Q_OBJECT ALBERT_PLUGIN
public:
    Plugin();
    ~Plugin();

    std::vector<albert::RankItem> handleGlobalQuery(const albert::Query *) const override;
    QWidget *buildConfigWidget() override;

private:
    struct Private;
    std::unique_ptr<Private> d;
};
