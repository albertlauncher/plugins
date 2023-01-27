// Copyright (c) 2022 Manuel Schneider

#pragma once
#include "albert.h"


class DictHandler final : public albert::QueryHandler
{
public:
    QString id() const override;
    QString name() const override;
    QString description() const override;
    QString synopsis() const override;
    QString defaultTrigger() const override;
    void handleQuery(Query &query) const override;
};


class Plugin : public albert::NativePluginInstance
{
    Q_OBJECT ALBERT_PLUGIN
public:
    Plugin();
    ~Plugin() override;

protected:
    DictHandler dict_handler;
};
