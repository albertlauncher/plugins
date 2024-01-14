// Copyright (c) 2022 Manuel Schneider

#pragma once
#include "albert/extension/queryhandler/triggerqueryhandler.h"

class FilePathBrowser : public albert::TriggerQueryHandler
{
public:
    FilePathBrowser(bool &caseSensitive);
    std::vector<std::shared_ptr<albert::Item>> buildItems(const QString &input) const;
    bool allowTriggerRemap() const override;
    bool &caseSensitive;
};

class RootBrowser : public FilePathBrowser
{
public:
    RootBrowser(bool &caseSensitive);
    QString id() const override;
    QString name() const override;
    QString description() const override;
    QString defaultTrigger() const override;
    void handleTriggerQuery(TriggerQuery*) const override;
};

class HomeBrowser : public FilePathBrowser
{
public:
    HomeBrowser(bool &caseSensitive);
    QString id() const override;
    QString name() const override;
    QString description() const override;
    QString defaultTrigger() const override;
    void handleTriggerQuery(TriggerQuery*) const override;
};
