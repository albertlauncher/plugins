// Copyright (c) 2022 Manuel Schneider

#pragma once
#include "albert.h"

class Plugin :
        public albert::ExtensionPlugin,
        public albert::TriggerQueryHandler
{
    Q_OBJECT ALBERT_PLUGIN
public:
    Plugin();
    void handleTriggerQuery(TriggerQuery&) const override;
    QWidget* buildConfigWidget() override;
};
