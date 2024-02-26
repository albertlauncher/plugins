// Copyright (c) 2023-2024 Manuel Schneider

#include "plugin.h"
#include "ui_configwidget.h"

namespace {
static const char* K_WND_POS = "window_position";
}

Plugin::Plugin() : qml_interface_(this), window(qml_interface_)
{
    auto s = settings();
    restore_always_on_top(s);
    restore_clear_on_hide(s);
    restore_display_system_shadow(s);
    restore_follow_mouse(s);
    restore_hide_on_close(s);
    restore_hide_on_focus_loss(s);
    restore_show_centered(s);

    s = state();
    window.setPosition(s->value(K_WND_POS).toPoint());

    connect(&window, &Window::inputChanged, this, &Plugin::inputChanged);
    connect(&window, &Window::visibleChanged, this, &Plugin::visibleChanged);
}

Plugin::~Plugin()
{
    state()->setValue(K_WND_POS, window.position());
}

bool Plugin::isVisible() const { return window.isVisible(); }

void Plugin::setVisible(bool visible) { window.setVisible(visible); }

QString Plugin::input() const { return window.input(); }

void Plugin::setInput(const QString &input) { window.setInput(input); }

unsigned long long Plugin::winId() const { return window.winId(); }

QWidget* Plugin::createFrontendConfigWidget()
{
    auto *w = new QWidget;
    Ui::ConfigWidget ui;
    ui.setupUi(w);

    ALBERT_PLUGIN_PROPERTY_CONNECT(this, always_on_top, ui.checkBox_onTop, setChecked, toggled)
    ALBERT_PLUGIN_PROPERTY_CONNECT(this, clear_on_hide, ui.checkBox_clearOnHide, setChecked, toggled)
    ALBERT_PLUGIN_PROPERTY_CONNECT(this, display_system_shadow, ui.checkBox_systemShadow, setChecked, toggled)
    ALBERT_PLUGIN_PROPERTY_CONNECT(this, follow_mouse, ui.checkBox_followMouse, setChecked, toggled)
    ALBERT_PLUGIN_PROPERTY_CONNECT(this, hide_on_close, ui.checkBox_hideOnClose, setChecked, toggled)
    ALBERT_PLUGIN_PROPERTY_CONNECT(this, hide_on_focus_loss, ui.checkBox_hideOnFocusOut, setChecked, toggled)
    ALBERT_PLUGIN_PROPERTY_CONNECT(this, show_centered, ui.checkBox_center, setChecked, toggled)

    // Themes

    // auto fillThemesCheckBox = [this, cb=ui.comboBox_themes](){
    //     QSignalBlocker b(cb);
    //     cb->clear();
    //     QStandardItemModel *model = qobject_cast<QStandardItemModel*>(cb->model());  // safe, see docs

    //     // Add disabled placeholder item
    //     auto *item = new QStandardItem;
    //     item->setText("Choose theme...");
    //     item->setEnabled(false);
    //     model->appendRow(item);

    //     cb->insertSeparator(1);

    //     // Add themes
    //     for (const QFileInfo &fi : availableThemes()){
    //         item = new QStandardItem;
    //         item->setText(fi.baseName());
    //         item->setToolTip(fi.absoluteFilePath());
    //         model->appendRow(item);
    //     }
    // };

    // fillThemesCheckBox();

    // connect(ui.comboBox_themes, &QComboBox::currentIndexChanged,
    //         this, [this, cb=ui.comboBox_themes](int i){
    //             auto theme_file_name = cb->model()->index(i,0).data(Qt::ToolTipRole).toString();
    //             applyTheme(theme_file_name);
    //         });

    // connect(ui.toolButton_propertyEditor, &QToolButton::clicked, this, [this, w](){
    //     PropertyEditor *pe = new PropertyEditor(this, w);
    //     pe->setWindowModality(Qt::WindowModality::WindowModal);
    //     pe->show();
    // });

    // connect(ui.toolButton_save, &QToolButton::clicked, this, [this, w, fillThemesCheckBox](){
    //     if (auto text = QInputDialog::getText(w, qApp->applicationDisplayName(), "Theme name:"); !text.isNull()){
    //         if (text.isEmpty())
    //             QMessageBox::warning(w, qApp->applicationDisplayName(), "Theme name must not be empty.");
    //         else if (auto dir = configDir(); dir.exists(text+".theme"))
    //             QMessageBox::warning(w, qApp->applicationDisplayName(), "Theme already exists.");
    //         else{
    //             saveThemeAsFile(dir.filePath(text));
    //             fillThemesCheckBox();
    //         }
    //     }
    // });


    return w;
}

void Plugin::setQuery(albert::Query *query) { qml_interface_.setQuery(query); }

bool Plugin::always_on_top() const
{ return window.flags() & Qt::WindowStaysOnTopHint; }

void Plugin::set_always_on_top_(bool value)
{ window.setFlags(window.flags().setFlag(Qt::WindowStaysOnTopHint, value)); }

bool Plugin::display_system_shadow() const
{ return !window.flags().testFlag(Qt::NoDropShadowWindowHint); }

void Plugin::set_display_system_shadow_(bool value)
{ window.setFlags(window.flags().setFlag(Qt::NoDropShadowWindowHint, !value)); }





