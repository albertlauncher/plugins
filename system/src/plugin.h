// Copyright (c) 2022 Manuel Schneider

#pragma once
#include "albert.h"

class Plugin : public albert::ExtensionPlugin,
               public albert::IndexQueryHandler
{
    Q_OBJECT ALBERT_PLUGIN
public:
    void updateIndexItems() override;
    QWidget* buildConfigWidget() override;
};
