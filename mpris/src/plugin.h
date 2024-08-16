// Copyright (c) 2017-2024 Manuel Schneider

#pragma once
#include <albert/extensionplugin.h>
#include <albert/globalqueryhandler.h>

class Plugin : public albert::ExtensionPlugin,
               public albert::GlobalQueryHandler
{
    ALBERT_PLUGIN

public:

    Plugin();
    ~Plugin();

    std::vector<albert::RankItem> handleGlobalQuery(const albert::Query *) override;
    QWidget *buildConfigWidget() override;

private:

    void serviceOwnerChanged(const QString&, const QString&, const QString&);

    struct Private;
    std::unique_ptr<Private> d;

};
