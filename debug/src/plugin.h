// Copyright (c) 2022 Manuel Schneider
#pragma once
#include "albert.h"

class Plugin final :
        public albert::ExtensionPlugin,
        public albert::TriggerQueryHandler
{
    Q_OBJECT ALBERT_PLUGIN
public:
    Plugin();
    ~Plugin() override;

    QString name() const override;
    QString description() const override;
    bool allowTriggerRemap() const override;
    QString synopsis() const override;
    void handleTriggerQuery(TriggerQuery&) const override;
};
