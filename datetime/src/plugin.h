// Copyright (c) 2023 Manuel Schneider

#pragma once
#include "albert.h"

class TimeZoneHandler final : public albert::QueryHandler
{
public:
    QString id() const override;
    QString name() const override;
    QString description() const override;
    QString synopsis() const override;
    QString defaultTrigger() const override;
    void handleQuery(Query &query) const override;
};


class Plugin final : public albert::ExtensionPlugin,
                     public albert::GlobalQueryHandler
{
    Q_OBJECT ALBERT_PLUGIN
public:
    Plugin();
    ~Plugin();

    std::vector<albert::RankItem> handleQuery(const Query&) const override;
private:
    TimeZoneHandler tzh;
};
