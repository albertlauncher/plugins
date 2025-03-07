// Copyright (c) 2022-2025 Manuel Schneider

#include "plugin.h"
#include "ui_configwidget.h"
#include <albert/albert.h>
#include <albert/extensionregistry.h>
#include <albert/logging.h>
ALBERT_LOGGING_CATEGORY("wbm")
using namespace albert;
using namespace std;


Plugin::Plugin() : window(this), themes_query_handler(&window)
{
    connect(&window, &Window::inputChanged, this, &Plugin::inputChanged);
    connect(&window, &Window::visibleChanged, this, &Plugin::visibleChanged);
}

vector<Extension *> Plugin::extensions()
{ return { &themes_query_handler }; }

QString Plugin::input() const
{ return window.input(); }

void Plugin::setInput(const QString &input)
{ window.setInput(input); }

bool Plugin::isVisible() const
{ return window.isVisible(); }

void Plugin::setVisible(bool visible)
{ window.setVisible(visible); }

template<typename T>
static void bind(T *t,
             QCheckBox *check_box,
             bool (T::*get)() const,
             void (T::*set)(bool),
             void (T::*sig)(bool))
{
    check_box->setChecked((t->*get)());
    QObject::connect(t, sig, check_box, &QCheckBox::setChecked);
    QObject::connect(check_box, &QCheckBox::toggled, t, set);
}

QWidget* Plugin::createFrontendConfigWidget()
{
    auto *widget = new QWidget;
    Ui::ConfigWidget ui;
    ui.setupUi(widget);

    for (const auto&[name, path] : window.themes)
    {
        ui.comboBox_theme_light->addItem(name, path);
        if (name == window.themeLight())
            ui.comboBox_theme_light->setCurrentIndex(ui.comboBox_theme_light->count()-1);
    }
    connect(ui.comboBox_theme_light,
            static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
            this, [this, comboBox_themes=ui.comboBox_theme_light](int i)
            { window.setThemeLight(comboBox_themes->itemText(i)); });
    connect(&window, &Window::themeLightChanged, widget, [cb=ui.comboBox_theme_light](QString theme){
        if (auto i = cb->findText(theme); i != -1)
            cb->setCurrentIndex(i);
    });

    for (const auto&[name, path] : window.themes)
    {
        ui.comboBox_theme_dark->addItem(name, path);
        if (name == window.themeDark())
            ui.comboBox_theme_dark->setCurrentIndex(ui.comboBox_theme_dark->count()-1);
    }
    connect(ui.comboBox_theme_dark,
            static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
            this, [this, comboBox_themes=ui.comboBox_theme_dark](int i)
            { window.setThemeDark(comboBox_themes->itemText(i)); });
    connect(&window, &Window::themeDarkChanged, widget, [cb=ui.comboBox_theme_dark](QString theme){
        if (auto i = cb->findText(theme); i != -1)
            cb->setCurrentIndex(i);
    });

    ::bind(&window, ui.checkBox_onTop,
           &Window::alwaysOnTop,
           &Window::setAlwaysOnTop,
           &Window::alwaysOnTopChanged);

    ::bind(&window, ui.checkBox_clearOnHide,
           &Window::clearOnHide,
           &Window::setClearOnHide,
           &Window::clearOnHideChanged);

    ::bind(&window, ui.checkBox_client_shadow,
           &Window::displayClientShadow,
           &Window::setDisplayClientShadow,
           &Window::displayClientShadowChanged);

    ::bind(&window, ui.checkBox_scrollbar,
           &Window::displayScrollbar,
           &Window::setDisplayScrollbar,
           &Window::displayScrollbarChanged);

    ::bind(&window, ui.checkBox_system_shadow,
           &Window::displaySystemShadow,
           &Window::setDisplaySystemShadow,
           &Window::displaySystemShadowChanged);

    ::bind(&window, ui.checkBox_followCursor,
           &Window::followCursor,
           &Window::setFollowCursor,
           &Window::followCursorChanged);

    ::bind(&window, ui.checkBox_hideOnFocusOut,
           &Window::hideOnFocusLoss,
           &Window::setHideOnFocusLoss,
           &Window::hideOnFocusLossChanged);

    ::bind(&window, ui.checkBox_history_search,
           &Window::historySearchEnabled,
           &Window::setHistorySearchEnabled,
           &Window::historySearchEnabledChanged);

    ui.spinBox_results->setValue((int)window.maxResults());
    connect(ui.spinBox_results, static_cast<void(QSpinBox::*)(int)>(&QSpinBox::valueChanged),
            &window, &Window::setMaxResults);
    connect(&window, &Window::maxResultsChanged,
            ui.spinBox_results, &QSpinBox::setValue);

    ::bind(&window, ui.checkBox_quit_on_close,
           &Window::quitOnClose,
           &Window::setQuitOnClose,
           &Window::quitOnCloseChanged);

    ::bind(&window, ui.checkBox_center,
           &Window::showCentered,
           &Window::setShowCentered,
           &Window::showCenteredChanged);

    ::bind(&window, ui.checkBox_debug,
           &Window::debugMode,
           &Window::setDebugMode,
           &Window::debugModeChanged);

    return widget;
}

unsigned long long Plugin::winId() const
{ return window.winId(); }

void Plugin::setQuery(Query *q)
{ window.setQuery(q); }
