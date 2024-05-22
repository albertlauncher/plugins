// Copyright (C) 2024 Manuel Schneider

#pragma once
#include "albert/query/globalqueryhandler.h"
#include "albert/util/extensionplugin.h"

class Plugin : public albert::ExtensionPlugin,
               public albert::GlobalQueryHandler
{
    Q_OBJECT ALBERT_PLUGIN
public:
    Plugin();
    ~Plugin();

    QString defaultTrigger() const override;
    std::vector<albert::RankItem> handleGlobalQuery(const albert::Query*) const override;
    QWidget *buildConfigWidget() override;

private:
    class Private;
    std::unique_ptr<Private> d;

};
