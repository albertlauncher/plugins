// Copyright (c) 2022-2024 Manuel Schneider

#include "plugin.h"
#include "ui_configwidget.h"
#include <albert/extensionregistry.h>
#include <albert/logging.h>
ALBERT_LOGGING_CATEGORY("wbm")
using namespace albert;
using namespace std;


Plugin::Plugin() : window(this), themes_query_handler(&window)
{
    connect(&window, &Window::inputChanged, this, &Plugin::inputChanged);
    connect(&window, &Window::visibleChanged, this, &Plugin::visibleChanged);
    registry().registerExtension(&themes_query_handler);
}

Plugin::~Plugin()
{
    registry().deregisterExtension(&themes_query_handler);
}

QString Plugin::input() const
{ return window.input(); }

void Plugin::setInput(const QString &input)
{ window.setInput(input); }

bool Plugin::isVisible() const
{ return window.isVisible(); }

void Plugin::setVisible(bool visible)
{ window.setVisible(visible); }

QWidget* Plugin::createFrontendConfigWidget()
{
    auto widget = new QWidget;
    Ui::ConfigWidget ui;
    ui.setupUi(widget);

    for (const auto&[name, path] : window.themes)
    {
        ui.comboBox_theme_light->addItem(name, path);
        if (name == window.lightTheme())
            ui.comboBox_theme_light->setCurrentIndex(ui.comboBox_theme_light->count()-1);
    }
    connect(ui.comboBox_theme_light,
            static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
            this, [this, comboBox_themes=ui.comboBox_theme_light](int i)
            { window.setLightTheme(comboBox_themes->itemText(i)); });

    for (const auto&[name, path] : window.themes)
    {
        ui.comboBox_theme_dark->addItem(name, path);
        if (name == window.darkTheme())
            ui.comboBox_theme_dark->setCurrentIndex(ui.comboBox_theme_dark->count()-1);
    }
    connect(ui.comboBox_theme_dark,
            static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
            this, [this, comboBox_themes=ui.comboBox_theme_dark](int i)
            { window.setDarkTheme(comboBox_themes->itemText(i)); });

    ui.checkBox_onTop->setChecked(window.alwaysOnTop());
    connect(ui.checkBox_onTop, &QCheckBox::toggled,
            &window, &Window::setAlwaysOnTop);

    ui.checkBox_clearOnHide->setChecked(window.clearOnHide());
    connect(ui.checkBox_clearOnHide, &QCheckBox::toggled,
            &window, &Window::setClearOnHide);

    ui.checkBox_client_shadow->setChecked(window.displayClientShadow());
    connect(ui.checkBox_client_shadow, &QCheckBox::toggled,
            &window, &Window::setDisplayClientShadow);

    ui.checkBox_scrollbar->setChecked(window.displayScrollbar());
    connect(ui.checkBox_scrollbar, &QCheckBox::toggled,
            &window, &Window::setDisplayScrollbar);

    ui.checkBox_system_shadow->setChecked(window.displaySystemShadow());
    connect(ui.checkBox_system_shadow, &QCheckBox::toggled,
            &window, &Window::setDisplaySystemShadow);

    ui.checkBox_followCursor->setChecked(window.followCursor());
    connect(ui.checkBox_followCursor, &QCheckBox::toggled,
            &window, &Window::setFollowCursor);

    ui.checkBox_hideOnFocusOut->setChecked(window.hideOnFocusLoss());
    connect(ui.checkBox_hideOnFocusOut, &QCheckBox::toggled,
            &window, &Window::setHideOnFocusLoss);

    ui.checkBox_history_search->setChecked(window.historySearchEnabled());
    connect(ui.checkBox_history_search, &QCheckBox::toggled,
            &window, &Window::setHistorySearchEnabled);

    ui.spinBox_results->setValue((int)window.maxResults());
    connect(ui.spinBox_results, static_cast<void(QSpinBox::*)(int)>(&QSpinBox::valueChanged),
            &window, &Window::setMaxResults);

    ui.checkBox_quit_on_close->setChecked(window.quitOnClose());
    connect(ui.checkBox_quit_on_close, &QCheckBox::toggled,
            &window, &Window::setQuitOnClose);

    ui.checkBox_center->setChecked(window.showCentered());
    connect(ui.checkBox_center, &QCheckBox::toggled,
            &window, &Window::setShowCentered);

    return widget;
}

unsigned long long Plugin::winId() const
{ return window.winId(); }

void Plugin::setQuery(Query *q)
{ window.setQuery(q); }
