// Copyright (c) 2023-2024 Manuel Schneider

#pragma once
#include "qmlinterface.h"
#include "window.h"
#include <QSettings>
#include <albert/frontend.h>
#include <albert/plugininstance.h>
#include <albert/property.h>

class Plugin : public albert::Frontend, public albert::PluginInstance
{
    ALBERT_PLUGIN

public:

    Plugin();
    ~Plugin();

    // albert::Frontend
    bool isVisible() const override;
    void setVisible(bool visible) override;
    QString input() const override;
    void setInput(const QString&) override;
    unsigned long long winId() const override;
    QWidget* createFrontendConfigWidget() override;
    void setQuery(albert::Query *query) override;

protected:

    QmlInterface qml_interface_;
    Window window;

    ALBERT_PLUGIN_PROPERTY_GETSET(bool, always_on_top, true)
    ALBERT_PLUGIN_PROPERTY_MEMBER(bool, clear_on_hide, window.clear_on_hide, true)
    ALBERT_PLUGIN_PROPERTY_GETSET(bool, display_system_shadow, true)
    ALBERT_PLUGIN_PROPERTY_MEMBER(bool, follow_mouse, window.follow_mouse, true)
    ALBERT_PLUGIN_PROPERTY_MEMBER(bool, hide_on_close, window.hide_on_close, true)
    ALBERT_PLUGIN_PROPERTY_MEMBER(bool, hide_on_focus_loss, window.hide_on_focus_loss, true)
    ALBERT_PLUGIN_PROPERTY_MEMBER(bool, show_centered, window.show_centered, true)

};


