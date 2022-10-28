// Copyright (c) 2022 Manuel Schneider

#pragma once
#include "albert.h"

class Plugin :
        public albert::ExtensionPlugin,
        public albert::QueryHandler,
        public albert::ConfigWidgetProvider
{
    Q_OBJECT ALBERT_PLUGIN
public:
    Plugin();
    void handleQuery(Query&) const override;
    QWidget* buildConfigWidget() override;
};
