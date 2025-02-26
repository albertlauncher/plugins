// Copyright (c) 2017-2024 Manuel Schneider

#pragma once
#include <QRegularExpression>
#include <QSet>
#include <QString>
#include <albert/extensionplugin.h>
#include <albert/globalqueryhandler.h>
#include <albert/plugin/applications.h>
#include <albert/plugindependency.h>

class Plugin : public albert::ExtensionPlugin,
               public albert::GlobalQueryHandler
{
    ALBERT_PLUGIN

public:

    Plugin();
    QString synopsis(const QString&) const override;
    bool allowTriggerRemap() const override;
    void handleTriggerQuery(albert::Query&) override;
    std::vector<albert::RankItem> handleGlobalQuery(const albert::Query&) override;
    QWidget* buildConfigWidget() override;

private:

    std::vector<albert::RankItem> getItems(const QString &query, bool allowParams) const;

    albert::StrongDependency<applications::Plugin> apps{"applications"};
    QSet<QString> hosts;
    const QString tr_desc;
    const QString tr_conn;
    static const QRegularExpression regex_synopsis;
    static const QStringList icon_urls;

};
