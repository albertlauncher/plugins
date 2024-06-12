// Copyright (c) 2022-2024 Manuel Schneider

#include "plugin.h"
#include "styleeditor.h"
#include "ui_configwidget.h"
#include <QPointer>
#include <QWindow>
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

    // openStyleEditor();
}

void Plugin::openStyleEditor()
{
    static QPointer<StyleEditor> instance;
    if (!instance)
        instance = new StyleEditor(&window);
    instance->setAttribute(Qt::WA_DeleteOnClose);
    // instance->showMaximized();
    instance->show();
}

QString Plugin::input() const
{ return window.input(); }

void Plugin::setInput(const QString &input)
{ window.setInput(input); }

bool Plugin::isVisible() const
{ return window.isVisible(); }

void Plugin::setVisible(bool visible)
{
    // Quick fix to prevent the flicker when the window is shown
    // due to the resize event being delivered on show. This enforces
    // the resize to happen before the window is hidden.
    if(!visible)
        setQuery(nullptr);
    QCoreApplication::processEvents();

    window.setVisible(visible);
}


#define conn(member, spinbox) \
spinbox->setValue(window.theme().member); \
    connect(spinbox, &QSpinBox::valueChanged, \
            this, [&](int v) { \
                    auto t = window.theme(); \
                    t.member = v; \
                    window.setTheme(t); \
                    window.update(); \
            });

// #define conn(member, brushbutton) \
// spinbox->setValue(window.theme().member); \
//     connect(spinbox, &QSpinBox::valueChanged, \
//             this, [&](int v) { \
//                     auto t = window.theme(); \
//                     t.member = v; \
//                     window.setTheme(t); \
//                     window.update(); \
//             });


QWidget* Plugin::createFrontendConfigWidget()
{
    auto widget = new QWidget;
    Ui::ConfigWidget ui;
    ui.setupUi(widget);

    auto w = &window;
    ALBERT_PROPERTY_CONNECT_CHECKBOX(w, always_on_top, ui.checkBox_onTop);
    ALBERT_PROPERTY_CONNECT_CHECKBOX(w, clear_on_hide, ui.checkBox_clearOnHide);
    ALBERT_PROPERTY_CONNECT_CHECKBOX(w, display_client_shadow, ui.checkBox_client_shadow);
    ALBERT_PROPERTY_CONNECT_CHECKBOX(w, display_system_shadow, ui.checkBox_system_shadow);
    ALBERT_PROPERTY_CONNECT_CHECKBOX(w, follow_cursor, ui.checkBox_followCursor);
    ALBERT_PROPERTY_CONNECT_CHECKBOX(w, hide_on_focus_loss, ui.checkBox_hideOnFocusOut);
    ALBERT_PROPERTY_CONNECT_CHECKBOX(w, history_search, ui.checkBox_history_search);
    ALBERT_PROPERTY_CONNECT_CHECKBOX(w, quit_on_close, ui.checkBox_quit_on_close);
    ALBERT_PROPERTY_CONNECT_CHECKBOX(w, show_centered, ui.checkBox_center);
    ALBERT_PROPERTY_CONNECT_SPINBOX(w, max_results, ui.spinBox_max_results);

    connect(ui.pushButton_styleEditor, &QPushButton::clicked, this,
            [this]{ auto *se = new StyleEditor(&window); se->show(); });









    // for (const auto&[name, path] : window.themes)
    // {
    //     ui.comboBox_theme_light->addItem(name, path);
    //     if (name == window.lightTheme())
    //         ui.comboBox_theme_light->setCurrentIndex(ui.comboBox_theme_light->count()-1);
    // }
    // connect(ui.comboBox_theme_light,
    //         static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
    //         this, [this, comboBox_themes=ui.comboBox_theme_light](int i)
    //         { window.setLightTheme(comboBox_themes->itemText(i)); });






    // for (const auto&[name, path] : window.themes)
    // {
    //     ui.comboBox_theme_dark->addItem(name, path);
    //     if (name == window.darkTheme())
    //         ui.comboBox_theme_dark->setCurrentIndex(ui.comboBox_theme_dark->count()-1);
    // }
    // connect(ui.comboBox_theme_dark,
    //         static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
    //         this, [this, comboBox_themes=ui.comboBox_theme_dark](int i)
    //         { window.setDarkTheme(comboBox_themes->itemText(i)); });




// #define setupSytleSpinBox(spinBoxPtr, windowPtr, styleMemberName) \
//     spinBoxPtr->setValue(windowPtr->style().styleMemberName); \
//     connect(spinBoxPtr, &QSpinBox::valueChanged, windowPtr, [w=windowPtr](int v){ \
//         auto s = w->style(); \
//         s.styleMemberName = v; \
//         w->setStyle(s); \
//     });

//     auto *wptr = &window;

    // setupSytleSpinBox(ui.spinBox_frame_width, wptr, frame_width);
    // setupSytleSpinBox(ui.spinBox_frame_border_radius, wptr, frame_border_radius);
    // setupSytleSpinBox(ui.spinBox_frame_border_width, wptr, frame_border_width);
    // setupSytleSpinBox(ui.spinBox_frame_padding, wptr, frame_padding);

    // setupSytleSpinBox(ui.spinBox_input_frame_border_radius, wptr, input_frame_border_radius);
    // setupSytleSpinBox(ui.spinBox_input_frame_border_width, wptr, input_frame_border_width);
    // setupSytleSpinBox(ui.spinBox_input_frame_padding, wptr, input_frame_padding);

    // setupSytleSpinBox(ui.spinBox_inputLine_fontSize, wptr, input_line_font_size);

    // setupSytleSpinBox(ui.spinBox_setting



    //     ui.spinBox_frame_width->setValue(window.style().frame_width);
    //     connect(ui.spinBox_frame_width, &QSpinBox::valueChanged,
    //             &window, [w=&window](int v){ auto s = w->style(); s.frame_width = v; w->setStyle(s); });

    //     spinbox->setValue(wptr->style().member);
    //     connect(spinbox, &QSpinBox::valueChanged, wptr, [w=wptr](int v){
    //         auto s = w->style();
    //         s.member = v;
    //         w->setStyle(s);
    //     });
    // };

    // ui.spinBox_frame_width->setValue(window.style().frame_width);
    // connect(ui.spinBox_frame_width, &QSpinBox::valueChanged,
    //         &window, [w=&window](int v){ auto s = w->style(); s.frame_width = v; w->setStyle(s); });


    // ALBERT_PROPERTY_CONNECT_SPINBOX(wptr, frame_padding, ui.spinBox_frame_padding);
    // ALBERT_PROPERTY_CONNECT_SPINBOX(wptr, frame_border_radius, ui.spinBox_frame_border_radius);
    // ALBERT_PROPERTY_CONNECT_SPINBOX(wptr, frame_border_width, ui.spinBox_frame_border_width);

    // ui.spinBox_inputLine_fontSize->setValue(window.input_line->font().pixelSize());
    // connect(ui.spinBox_inputLine_fontSize, &QSpinBox::valueChanged, this,
    //         [this](int v){
    //     // auto f = window.input_line->font();
    //     // f.setPixelSize(v);
    //     // window.input_line->setFont(f);
    //     // window.update();
    // });



    // #define conn(member, spinbox) \
    //     spinbox->setValue(window.theme().member); \
    //     connect(spinbox, &QSpinBox::valueChanged, this, \
    //             [&](int v){ member = v; window.update(); window.results_list->reset(); });

    //     conn(item_text_font_size, ui.spinBox_item_text_fontSize)

    // auto conn = [this](auto &member, auto *spinbox){
    //     spinbox->setValue(window.theme().member);
    //     connect(spinbox, &QSpinBox::valueChanged, this,
    //             [&](int v){ member = v; window.update(); window.results_list->reset(); });
    // };

    // conn(window.theme->item_text_font_size,     ui.spinBox_item_text_fontSize);
    // conn(window.theme->item_subtext_font_size,  ui.spinBox_item_subtext_fontSize);
    // conn(window.theme->item_padding,            ui.spinBox_item_padding);
    // conn(window.theme->item_icon_size,          ui.spinBox_icon_size);
    // conn(window.theme->item_horizonzal_spacing, ui.spinBox_item_hspace);
    // conn(window.theme->item_vertical_spacing,   ui.spinBox_item_vspace);






    // ui.checkBox_debug->setChecked(window.item_delegate->debug);
    // connect(ui.checkBox_debug, &QCheckBox::toggled, this,
    //         [this](int v){ window.item_delegate->debug = v; window.update(); });

    // auto *b = new BrushButton(QGradient(QGradient::JungleDay));
    // widget->layout()->addWidget(b);

    // ui.spinBox_frame_margins->setValue(window.frame->contentsMargins().top());
    // connect(ui.spinBox_frame_margins, &QSpinBox::valueChanged, this,
    //         [this](int v){ window.frame->setContentsMargins(v,v,v,v);  window.update(); });




    // conn(input_line_font_size, ui.spinBox_inputLine_fontSize)
    // conn(result_item_text_font_size, ui.spinBox_item_text_fontSize)
    // conn(result_item_subtext_font_size, ui.spinBox_item_subtext_fontSize)
    // conn(result_item_icon_size, ui.spinBox_icon_size)
    // conn(result_item_horizonzal_spacing, ui.spinBox_item_hspace)
    // conn(result_item_vertical_spacing, ui.spinBox_item_vspace)
    // conn(input_frame_padding, ui.spinBox_frame_margins)




    // auto bla = initializer_list<tuple<QString, QPalette::ColorRole>>{
    //     {tr("Window"), QPalette::Window},
    //     {tr("Window text"), QPalette::WindowText},
    //     {tr("Base"), QPalette::Base},
    //     {tr("Highlight"), QPalette::Highlight},
    //     {tr("Highlighted text"), QPalette::HighlightedText},
    //     {tr("Light"), QPalette::Light},
    //     {tr("Mid"), QPalette::Mid},
    //     {tr("Dark"), QPalette::Dark},
    // };

    // auto palette_setter = [this](const BrushButton *bb, QPalette::ColorRole role) {
    //     auto t = window.theme();
    //     t.palette.setBrush(role, bb->brush());
    //     window.setTheme(t);
    //     window.update();
    // };

    // auto &t = window.theme();
    // // auto *hl = new QHBoxLayout;
    // for (const auto &[label, role] : bla)
    // {
    //     auto *bb = new BrushButton(t.palette.brush(role), widget);
    //     bb->setToolTip(label);
    //     connect(bb, &BrushButton::brushChanged, this, [=]{ palette_setter(bb, role); });
    //     ui.formLayout_appearance->addRow(label, bb);
    // }



    return widget;
}

unsigned long long Plugin::winId() const
{ return window.winId(); }

void Plugin::setQuery(Query *q)
{ window.setQuery(q); }
