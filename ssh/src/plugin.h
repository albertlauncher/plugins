// Copyright (c) 2017-2024 Manuel Schneider

#pragma once
#include "albert/extension/queryhandler/globalqueryhandler.h"
#include "albert/plugin.h"
#include <QString>
#include <QSet>
#include <memory>
#include <QRegularExpression>
namespace albert {
class StandardItem;
}

class Plugin : public albert::plugin::ExtensionPlugin,
               public albert::GlobalQueryHandler
{
    Q_OBJECT ALBERT_PLUGIN
public:
    Plugin();

    QString synopsis() const override;
    std::vector<albert::RankItem> handleGlobalQuery(const GlobalQuery *) const override;
    QWidget* buildConfigWidget() override;

private:
    QSet<QString> hosts_;
    static const QRegularExpression re_input;
    static const QStringList icon_urls;
};
