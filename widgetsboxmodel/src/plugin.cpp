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
                 void (T::*set)(bool))
{
    check_box->setChecked((t->*get)());
    QObject::connect(check_box, &QCheckBox::toggled, t, set);
}

template<typename T>
static void bind(T *t,
                 QCheckBox *check_box,
                 bool (T::*get)() const,
                 void (T::*set)(bool),
                 void (T::*sig)(bool))
{
    bind(t, check_box, get, set);
    QObject::connect(t, sig, check_box, &QCheckBox::setChecked);
}

template<typename T>
static QSpinBox *createSpinBox(QFormLayout *form_layout,
                               QString label,
                               T *t,
                               uint (T::*get)() const,
                               void (T::*set)(uint))
{
    auto *spin_box = new QSpinBox;
    spin_box->setValue((t->*get)());
    QObject::connect(spin_box, QOverload<int>::of(&QSpinBox::valueChanged), t, set);
    spin_box->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    form_layout->addRow(label, spin_box);
    return spin_box;
}

template<typename T>
static auto addFontSpinBox(QFormLayout *form_layout,
                           QString label,
                           T *t,
                           uint (T::*get)() const,
                           void (T::*set)(uint))
{
    auto *spin_box = createSpinBox(form_layout, label, t, get, set);
    spin_box->setMinimum(6);
    spin_box->setSuffix(QStringLiteral(" pt"));
    return spin_box;
}

template<typename T>
static auto addPixelMetricSpinBox(QFormLayout *form_layout,
                                  QString label,
                                  T *t,
                                  uint (T::*get)() const,
                                  void (T::*set)(uint))
{
    auto *spin_box = createSpinBox(form_layout, label, t, get, set);
    spin_box->setSuffix(QStringLiteral(" px"));
    return spin_box;
}

template<typename T>
static auto addPixelMetricSpinBox(QFormLayout *form_layout,
                                 QString label,
                                 T *t,
                                 double (T::*get)() const,
                                 void (T::*set)(double))
{

    auto *spin_box = new QDoubleSpinBox;
    spin_box->setValue((t->*get)());
    spin_box->setSingleStep(0.5);
    spin_box->setDecimals(1);
    spin_box->setSuffix(QStringLiteral(" px"));
    spin_box->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    QObject::connect(spin_box, QOverload<double>::of(&QDoubleSpinBox::valueChanged), t, set);
    form_layout->addRow(label, spin_box);
    return spin_box;
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

    ::bind(&window, ui.checkBox_input_method,
           &Window::disableInputMethod,
           &Window::setDisableInputMethod);

    ::bind(&window, ui.checkBox_center,
           &Window::showCentered,
           &Window::setShowCentered,
           &Window::showCenteredChanged);

    ::bind(&window, ui.checkBox_debug,
           &Window::debugMode,
           &Window::setDebugMode,
           &Window::debugModeChanged);

    auto *fl = ui.formLayout;
    QSpinBox *sb;

    addFontSpinBox(fl, tr("Input font size"), &window,
                   &Window::inputFontSize, &Window::setInputFontSize);

    addFontSpinBox(fl, tr("Result title font size"), &window,
                   &Window::resultItemTextFontSize, &Window::setResultItemTextFontSize);

    addFontSpinBox(fl, tr("Result description font size"), &window,
                   &Window::resultItemSubtextFontSize, &Window::setResultItemSubtextFontSize);

    addFontSpinBox(fl, tr("Action font size"), &window,
                   &Window::actionItemFontSize, &Window::setActionItemFontSize);


    sb = addPixelMetricSpinBox(fl, tr("Window width"), &window,
                               &Window::windowWidth, &Window::setWindowWidth);
    sb->setSingleStep(10);
    QSignalBlocker b(sb);  // setRange emits value change
    sb->setRange(100, 9999);

    addPixelMetricSpinBox(fl, tr("Window border radius"), &window,
                          &Window::windowBorderRadius, &Window::setWindowBorderRadius);

    addPixelMetricSpinBox(fl, tr("Window border width"), &window,
                          &Window::windowBorderWidth, &Window::setWindowBorderWidth);

    addPixelMetricSpinBox(fl, tr("Window padding"), &window,
                          &Window::windowPadding, &Window::setWindowPadding);

    addPixelMetricSpinBox(fl, tr("Window spacing"), &window,
                          &Window::windowSpacing, &Window::setWindowSpacing);


    addPixelMetricSpinBox(fl, tr("Input frame border radius"), &window,
                          &Window::inputBorderRadius, &Window::setInputBorderRadius);

    addPixelMetricSpinBox(fl, tr("Input frame border width"), &window,
                          &Window::inputBorderWidth, &Window::setInputBorderWidth);

    addPixelMetricSpinBox(fl, tr("Input frame padding"), &window,
                          &Window::inputPadding, &Window::setInputPadding);


    addPixelMetricSpinBox(fl, tr("Result item selection border radius"), &window,
                          &Window::resultItemSelectionBorderRadius, &Window::setResultItemSelectionBorderRadius);

    addPixelMetricSpinBox(fl, tr("Result item selection border width"), &window,
                          &Window::resultItemSelectionBorderWidth, &Window::setResultItemSelectionBorderWidth);

    addPixelMetricSpinBox(fl, tr("Result item padding"), &window,
                          &Window::resultItemPadding, &Window::setResultItemPadding);

    addPixelMetricSpinBox(fl, tr("Result item icon size"), &window,
                          &Window::resultItemIconSize, &Window::setResultItemIconSize);

    addPixelMetricSpinBox(fl, tr("Result item horizontal spacing"), &window,
                          &Window::resultItemHorizontalSpace, &Window::setResultItemHorizontalSpace);

    addPixelMetricSpinBox(fl, tr("Result item vertical spacing"), &window,
                          &Window::resultItemVerticalSpace, &Window::setResultItemVerticalSpace);


    addPixelMetricSpinBox(fl, tr("Action item selection border radius"), &window,
                          &Window::actionItemSelectionBorderRadius, &Window::setActionItemSelectionBorderRadius);

    addPixelMetricSpinBox(fl, tr("Action item selection border width"), &window,
                          &Window::actionItemSelectionBorderWidth, &Window::setActionItemSelectionBorderWidth);

    addPixelMetricSpinBox(fl, tr("Action item padding"), &window,
                          &Window::actionItemPadding, &Window::setActionItemPadding);




    return widget;
}

unsigned long long Plugin::winId() const
{ return window.winId(); }

void Plugin::setQuery(Query *q)
{ window.setQuery(q); }
