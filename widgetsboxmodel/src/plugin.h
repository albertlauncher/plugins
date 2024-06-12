// Copyright (c) 2022-2024 Manuel Schneider

#pragma once

#include "themesqueryhandler.h"
#include "window.h"
#include <QString>
#include <albert/frontend.h>
#include <albert/plugininstance.h>

class Plugin : public albert::Frontend,
               public albert::PluginInstance
{
    ALBERT_PLUGIN

public:

    Plugin();
    ~Plugin();

    void openStyleEditor();

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

// class PluginT : public albert::Frontend,
//                public albert::PluginInstance
// {
//     Q_OBJECT
//     ALBERT_PLUGIN

//     QWidget w;

// public:
//     // albert::Frontend
//     bool isVisible() const override { return w.isVisible(); }
//     void setVisible(bool visible) override { w.setVisible(visible); }
//     QString input() const override { return ""; }
//     void setInput(const QString&) override {}
//     QWidget* createFrontendConfigWidget() override { return nullptr; }
//     unsigned long long winId() const override { return w.winId(); }
//     void setQuery(albert::Query *query) override {}

// private:

// };
