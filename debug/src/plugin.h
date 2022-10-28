// Copyright (c) 2022 Manuel Schneider
#pragma once
#include "albert.h"

class Plugin final :
        public albert::ExtensionPlugin,
        public albert::QueryHandler
{
    Q_OBJECT ALBERT_PLUGIN
public:
    Plugin();
    ~Plugin() override;

    QString name() const override;
    QString description() const override;
    bool allowTriggerRemap() const override;
    QString synopsis() const override;
    void handleQuery(Query&) const override;
};
