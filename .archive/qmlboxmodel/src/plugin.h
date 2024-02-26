// Copyright (c) 2023-2024 Manuel Schneider

#pragma once
#include "albert/extension/frontend/frontend.h"
#include "albert/plugin.h"
#include "qmlinterface.h"
#include "window.h"
#include <QSettings>

class Plugin : public albert::Frontend, public albert::PluginInstance
{
    Q_OBJECT
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

    ALBERT_PLUGIN_PROPERTY_BASE(bool, always_on_top, true)
    bool always_on_top() const;
    void set_always_on_top_(bool value);
    ALBERT_PLUGIN_PROPERTY_MEMBER(bool, clear_on_hide, window.clear_on_hide, true)
    ALBERT_PLUGIN_PROPERTY_BASE(bool, display_system_shadow, true)
    bool display_system_shadow() const;
    void set_display_system_shadow_(bool value);
    ALBERT_PLUGIN_PROPERTY_MEMBER(bool, follow_mouse, window.follow_mouse, true)
    ALBERT_PLUGIN_PROPERTY_MEMBER(bool, hide_on_close, window.hide_on_close, true)
    ALBERT_PLUGIN_PROPERTY_MEMBER(bool, hide_on_focus_loss, window.hide_on_focus_loss, true)
    ALBERT_PLUGIN_PROPERTY_MEMBER(bool, show_centered, window.show_centered, true)

};


