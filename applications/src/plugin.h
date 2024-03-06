// Copyright (c) 2022-2024 Manuel Schneider

#pragma once
#include <albert/indexqueryhandler.h>
#include <albert/extensionplugin.h>
#if defined(Q_OS_UNIX) && !defined(Q_OS_MACOS)
#include <albert/property.h>
#endif
#include <memory>

class Plugin : public albert::ExtensionPlugin,
               public albert::IndexQueryHandler
{
    ALBERT_PLUGIN
public:
    Plugin();
    ~Plugin();

    QString defaultTrigger() const override;
    void updateIndexItems() override;
    QWidget *buildConfigWidget() override;

#if defined(Q_OS_MACOS)
#elif defined(Q_OS_UNIX)
    ALBERT_PLUGIN_PROPERTY(bool, ignore_show_in_keys, false)
    ALBERT_PLUGIN_PROPERTY(bool, use_exec, false)
    ALBERT_PLUGIN_PROPERTY(bool, use_generic_name, false)
    ALBERT_PLUGIN_PROPERTY(bool, use_keywords, false)
    ALBERT_PLUGIN_PROPERTY(bool, use_non_localized_name, false)
#elif defined(Q_OS_WIN)
#endif

protected:
    class Private;
    std::unique_ptr<Private> d;
};
