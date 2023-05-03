// Copyright (c) 2022 Manuel Schneider

#pragma once
#include "albert.h"

class AbstractBrowser : public albert::TriggerQueryHandler
{
public:
    QString description() const override;
    bool allowTriggerRemap() const override;
};

class RootBrowser : public AbstractBrowser
{
    QString id() const override;
    QString name() const override;
    QString defaultTrigger() const override;
    void handleTriggerQuery(TriggerQuery &query) const override;
};

class HomeBrowser : public AbstractBrowser
{
    QString id() const override;
    QString name() const override;
    QString defaultTrigger() const override;
    void handleTriggerQuery(TriggerQuery &query) const override;
};
