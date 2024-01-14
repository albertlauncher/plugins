// Copyright (c) 2022-2024 Manuel Schneider

#pragma once
#include "albert/extension/frontend/frontend.h"
#include "albert/plugin.h"
#include "themesqueryhandler.h"
#include "window.h"
#include <QString>
#include <map>

class Plugin : public albert::Frontend,
               public albert::PluginInstance
{
    Q_OBJECT
    ALBERT_PLUGIN

public:

    Plugin();

    // albert::Plugin
    void initialize(albert::ExtensionRegistry&, std::map<QString,PluginInstance*>) override;
    void finalize(albert::ExtensionRegistry &registry) override;

    // albert::Frontend
    bool isVisible() const override;
    void setVisible(bool visible) override;
    QString input() const override;
    void setInput(const QString&) override;
    QWidget* createFrontendConfigWidget() override;
    unsigned long long winId() const override;
    void setQuery(albert::Query *query) override;

private:

    Window window;
    ThemesQueryHandler themes_query_handler;

};
