#include "brushbutton.h"
#include "styleeditor.h"
#include "window.h"
#include <QCheckBox>
#include <QColorDialog>
#include <QFormLayout>
#include <QSpinBox>


StyleEditor::StyleEditor(Window *w, const QString &style_file) :
    window{w},
    style{style_file.isEmpty() ? w->style() : Style::read(style_file)}
{
    ui.setupUi(this);

#define createSpinBox(value) \
    sb = new QSpinBox(this); \
    sb->setValue(style.value); \
    connect(sb, QOverload<int>::of(&QSpinBox::valueChanged), this, [this](int size) { \
            style.value = size; \
            window->setStyle(style); \
    });

#define addFontSpinBox(form_layout, label, value) \
    createSpinBox(value) \
    sb->setMinimum(6); \
    sb->setSuffix(QStringLiteral(" pt")); \
    form_layout->addRow(label, sb);

#define addPixelMetricSpinBox(form_layout, label, value) \
    createSpinBox(value) \
    sb->setSuffix(QStringLiteral(" px")); \
    form_layout->addRow(label, sb);

#define addPaletteOverwriteBrushButton(form_layout, label, value) \
    bb = new BrushButton(style.value); \
    connect(bb, &BrushButton::brushChanged, this, [this, bb]() { \
            style.value = bb->brush(); \
            window->setStyle(style); \
    });\
    form_layout->addRow(label, bb);

#define addPaletteOverwriteColor(form_layout, label, value) \
    bb = new BrushButton(style.value); \
        connect(bb, &BrushButton::brushChanged, this, [this, bb]() { \
                style.value = bb->brush().color(); \
                window->setStyle(style); \
        });\
        form_layout->addRow(label, bb);

#define addStyleCheckbox(form_layout, label, value) \
    {\
        auto *_w = new QCheckBox; \
        _w->setChecked(style.value); \
        connect(_w, QOverload<int>::of(&QCheckBox::stateChanged), this, [this](int state) { \
                style.value = (bool)state; \
                window->setStyle(style); \
        });\
        form_layout->addRow(label, _w); \
    }


#define addPaletteBrushRow(form_layout, label, role) { \
    auto *lhl = new QHBoxLayout(ui.tab_palette); \
    bb = new BrushButton(style.palette.brush(QPalette::Active, QPalette::role)); \
        connect(bb, &BrushButton::brushChanged, this, [this, r=QPalette::role, bb]() { \
                style.palette.setBrush(QPalette::Active, r, bb->brush()); \
                window->setStyle(style); \
        }); \
    lhl->addWidget(bb); \
    bb = new BrushButton(style.palette.brush(QPalette::Disabled, QPalette::role)); \
        connect(bb, &BrushButton::brushChanged, this, [this, r=QPalette::role, bb]() { \
                style.palette.setBrush(QPalette::Disabled, r, bb->brush()); \
                window->setStyle(style); \
        }); \
    lhl->addWidget(bb); \
    bb = new BrushButton(style.palette.brush(QPalette::Inactive, QPalette::role)); \
        connect(bb, &BrushButton::brushChanged, this, [this, r=QPalette::role, bb]() { \
                style.palette.setBrush(QPalette::Inactive, r, bb->brush()); \
                window->setStyle(style); \
        }); \
    lhl->addWidget(bb); \
    form_layout->addRow(label, lhl); }


    BrushButton *bb;

    auto *hl = new QHBoxLayout(ui.tab_palette);
    ui.verticalLayout_palette->addLayout(hl);

    auto *fl = new QFormLayout;
    hl->addLayout(fl);
    addPaletteBrushRow(fl, tr("Window"), Window)
    addPaletteBrushRow(fl, tr("Window text"), WindowText)
    addPaletteBrushRow(fl, tr("Base"), Base)
    addPaletteBrushRow(fl, tr("Alternate base"), AlternateBase)
    addPaletteBrushRow(fl, tr("Text"), Text)
    addPaletteBrushRow(fl, tr("Tooltip base"), ToolTipBase)
    addPaletteBrushRow(fl, tr("Tooltip Text"), ToolTipText)
    addPaletteBrushRow(fl, tr("Button"), Button)
    addPaletteBrushRow(fl, tr("Button text"), ButtonText)
    addPaletteBrushRow(fl, tr("Bright text"), BrightText)

    fl = new QFormLayout;
    hl->addLayout(fl);
    addPaletteBrushRow(fl, tr("Highlight"), Highlight)
    addPaletteBrushRow(fl, tr("Highlighted text"), HighlightedText)
    addPaletteBrushRow(fl, tr("Link"), Link)
    addPaletteBrushRow(fl, tr("Visited link"), LinkVisited)
    addPaletteBrushRow(fl, tr("Placeholder text"), PlaceholderText)
    addPaletteBrushRow(fl, tr("Light"), Light)
    addPaletteBrushRow(fl, tr("Midlight"), Midlight)
    addPaletteBrushRow(fl, tr("Mid"), Mid)
    addPaletteBrushRow(fl, tr("Dark"), Dark)
    addPaletteBrushRow(fl, tr("Shadow"), Shadow)




    // Palette overwrites
    hl = new QHBoxLayout(ui.tab_window);
    ui.verticalLayout_window->addLayout(hl);
    fl = new QFormLayout(ui.tab_window);
    hl->addLayout(fl);
    addPaletteOverwriteBrushButton(fl, tr("Window background"), window_background_brush)
    addPaletteOverwriteBrushButton(fl, tr("Window border"), window_border_brush)
    addPaletteOverwriteBrushButton(fl, tr("Input frame border"), input_frame_border_brush)
    addPaletteOverwriteBrushButton(fl, tr("Input frame background"), input_frame_background_brush)
    addPaletteOverwriteColor(fl, tr("Settings button"), settings_button_color)
    addPaletteOverwriteColor(fl, tr("Settings button highlight"), settings_button_highlight_color)
    fl = new QFormLayout(ui.tab_window);
    hl->addLayout(fl);
    addPaletteOverwriteBrushButton(fl, tr("Item view background"), item_view_background_brush)
    addPaletteOverwriteBrushButton(fl, tr("Item view border"), item_view_border_brush)
    addPaletteOverwriteBrushButton(fl, tr("Item view item selection background"), item_view_item_selection_background_brush)
    addPaletteOverwriteBrushButton(fl, tr("Item view item selection border"), item_view_item_selection_border_brush)
    addPaletteOverwriteBrushButton(fl, tr("Result item text"), result_item_text_color)
    addPaletteOverwriteBrushButton(fl, tr("Result item subtext"), result_item_subtext_color)
    addPaletteOverwriteBrushButton(fl, tr("Action item text"), action_item_text_color)

    // Metrics
    hl = new QHBoxLayout(ui.tab_metrics);
    ui.verticalLayout_metrics->addLayout(hl);
    fl = new QFormLayout(ui.tab_metrics);
    hl->addLayout(fl);
    QSpinBox *sb;
    addFontSpinBox(fl, tr("Input font size"), input_line_font_size);
    addFontSpinBox(fl, tr("Result title font size"), result_item_text_font_size);
    addFontSpinBox(fl, tr("Result description font size"), result_item_subtext_font_size);
    addFontSpinBox(fl, tr("Action font size"), action_item_font_size);
    // Frame width needs extra treatment
    {
        sb = new QSpinBox(this);
        sb->setSingleStep(10);
        sb->setRange(100, 9999);
        sb->setValue(style.window_width);
        connect(sb, QOverload<int>::of(&QSpinBox::valueChanged), this, [this](int size) {
                style.window_width = size;
                window->setStyle(style);
        });
        sb->setSuffix(QStringLiteral(" px"));
        fl->addRow(tr("Window width"), sb);
    }
    addPixelMetricSpinBox(fl, tr("Window border radius"), window_border_radius);
    addPixelMetricSpinBox(fl, tr("Window border width"), window_border_width);
    addPixelMetricSpinBox(fl, tr("Window padding"), window_padding);
    addPixelMetricSpinBox(fl, tr("Window shadow size"), window_shadow_size)
    addPixelMetricSpinBox(fl, tr("Window shadow offset"), window_shadow_voffset)
    addPixelMetricSpinBox(fl, tr("Input frame border radius"), input_frame_border_radius);
    addPixelMetricSpinBox(fl, tr("Input frame border width"), input_frame_border_width);
    addPixelMetricSpinBox(fl, tr("Input frame padding"), input_frame_padding);
    fl = new QFormLayout(ui.tab_metrics);
    hl->addLayout(fl);
    addPixelMetricSpinBox(fl, tr("Settings button size"), settings_button_size);
    addPixelMetricSpinBox(fl, tr("Item view border radius"), item_view_border_radius);
    addPixelMetricSpinBox(fl, tr("Item view border width"), item_view_border_width);
    addPixelMetricSpinBox(fl, tr("Item view padding"), item_view_padding);
    addPixelMetricSpinBox(fl, tr("Item view item selection border radius"), item_view_item_selection_border_radius);
    addPixelMetricSpinBox(fl, tr("Item view item selection border width"), item_view_item_selection_border_width);
    addPixelMetricSpinBox(fl, tr("Item view item padding"), item_view_item_padding);
    addPixelMetricSpinBox(fl, tr("Result item icon size"), result_item_icon_size);
    addPixelMetricSpinBox(fl, tr("Result item horizontal spacing"), result_item_horizontal_spacing);
    addPixelMetricSpinBox(fl, tr("Result item vertical spacing"), result_item_vertical_spacing);
    addStyleCheckbox(fl, tr("Draw debug overlay"), draw_debug_overlays);


    // Styles
    initStylesGroupBox();

    connect(ui.pushButton_palettefromColor, &QAbstractButton::clicked,
        this, &StyleEditor::onChoosePaletteFromColor);


    // ui.horizontalLayout_style->addWidget(window);
    // show();
    // window->setInput("albert ");

}

StyleEditor::~StyleEditor()
{
    // ui.horizontalLayout_style->removeWidget(window);
    // window->setParent(nullptr);
}

void StyleEditor::onChoosePaletteFromColor()
{

    const auto c = QColorDialog::getColor(style.palette.button().color(),
                                                 this, {}, QColorDialog::ShowAlphaChannel);
    if (c.isValid()) {
        style.palette = QPalette(c);
        window->setStyle(style);
    }
}

void StyleEditor::onChoosePaletteFromStyle()
{


}


void StyleEditor::keyPressEvent(QKeyEvent *event)
{
    if (event->modifiers() == Qt::NoModifier){
        if (event->key() == Qt::Key_Escape)
            close();
    }

    else if (event->modifiers() == Qt::ControlModifier){
        if(event->key() == Qt::Key_W)
            close();
    }

    QWidget::keyPressEvent(event);
}

void StyleEditor::initStylesGroupBox()
{
    ui.comboBox_style->addItem(tr("Default"), "");
    ui.comboBox_style->setItemData(0, tr("Built in style"), Qt::ToolTipRole);
    ui.comboBox_style->insertSeparator(ui.comboBox_style->count());

    for (const auto &fi : window->findStyles())
    {
        ui.comboBox_style->insertItem(ui.comboBox_style->count(),
                                      fi.baseName(),
                                      fi.canonicalFilePath());
        ui.comboBox_style->setItemData(ui.comboBox_style->count()-1,
                                       fi.canonicalFilePath(), Qt::ToolTipRole);
    }

    connect(ui.comboBox_style, QOverload<int>::of(&QComboBox::activated),
            this, [this](int index){ setStyleFile(ui.comboBox_style->itemData(index).toString());});

}

void StyleEditor::setStyleFile(const QString &path)
{
    auto fi = QFileInfo(path);
    auto di = QFileInfo(fi.absolutePath());

    // ui.horizontalLayout_style->setEnabled(fi.isWritable());
    // ui.pushButton_remove->setEnabled(di.isWritable());

    // Style
    //     if (haveDarkSystemPalette())

    // ui.radioButton_dark->setChecked(true);
    // else
    //     ui.radioButton_light->setChecked(true);

}
