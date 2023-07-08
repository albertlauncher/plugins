// Copyright (c) 2022 Manuel Schneider

#pragma once
#include "albert/extension/queryhandler/triggerqueryhandler.h"

class AbstractBrowser : public albert::TriggerQueryHandler
{
public:
    AbstractBrowser(bool &caseSensitive);
    std::vector<std::shared_ptr<albert::Item>> buildItems(const QString &input) const;
    QString description() const override;
    bool allowTriggerRemap() const override;
    bool &caseSensitive;
};

class RootBrowser : public AbstractBrowser
{
public:
    RootBrowser(bool &caseSensitive);
    QString id() const override;
    QString name() const override;
    QString defaultTrigger() const override;
    void handleTriggerQuery(TriggerQuery*) const override;
};

class HomeBrowser : public AbstractBrowser
{
public:
    HomeBrowser(bool &caseSensitive);
    QString id() const override;
    QString name() const override;
    QString defaultTrigger() const override;
    void handleTriggerQuery(TriggerQuery*) const override;
};
