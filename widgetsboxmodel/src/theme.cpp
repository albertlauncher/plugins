// Copyright (c) 2024-2025 Manuel Schneider

#include "theme.h"
#include <QApplication>
#include <QBrush>
#include <QColor>
#include <QDir>
#include <QFont>
#include <QMetaEnum>
#include <QPalette>
#include <QSettings>
#include <QStyle>
#include <albert/logging.h>
using namespace std;

namespace
{

struct {
const char *window_shadow_brush = "window_shadow_brush";
const char *window_background_brush = "window_background_brush";
const char *window_border_brush = "window_border_brush";
const char *input_background_brush = "input_background_brush";
const char *input_border_brush = "input_border_brush";
const char *settings_button_color = "settings_button_color";
const char *settings_button_highlight_color = "settings_button_highlight_color";
const char *result_item_selection_background_brush = "result_item_selection_background_brush";
const char *result_item_selection_border_brush = "result_item_selection_border_brush";
const char *result_item_selection_text_color = "result_item_selection_text_color";
const char *result_item_selection_subtext_color = "result_item_selection_subtext_color";
const char *result_item_text_color = "result_item_text_color";
const char *result_item_subtext_color = "result_item_subtext_color";
const char *action_item_selection_background_brush = "action_item_selection_background_brush";
const char *action_item_selection_border_brush = "action_item_selection_border_brush";
const char *action_item_selection_text_color = "action_item_selection_text_color";
const char *action_item_text_color = "action_item_text_color";
} keys;

}

template<typename QEnum>
static const char *toString(const QEnum value)
{ return QMetaEnum::fromType<QEnum>().valueToKey(value); }

static QString toString(const QColor &color)
{ return color.name(color.alpha() == 255 ? QColor::HexRgb : QColor::HexArgb); }

static QString toString(const QBrush &brush)
{
    // if (auto img = brush.textureImage(); !img.isNull())
    // {´
    //     return "img";
    // }
    // else if (auto color = brush.color(); color != QColor::Invalid)
    // {
    //     return toString(color);
    // }
    // else if (auto gradient = brush.gradient(); gradient != nullptr)
    // {
    //     if (gradient->type() == QGradient::LinearGradient)
    //     {

    //     }
    //     QString str;
    //     gradient->stops();

    //     for (const QGradientStop &stop : gradient->stops())
    //         str += toString(stop.second) + " ";

    //     return "some other gradient";
    // }
    // else
        return {};
}

// static QColor parseColor(const QString &)
// {
//     QColor c = QColor::fromString(s);
//     if (!c.isValid())
//         WARN << "Failed to parse color:" << s;
// }



static const array<QPalette::ColorGroup, 3> colorGroups
{
    QPalette::Active,
    QPalette::Inactive,
    QPalette::Disabled
};

static const array<QPalette::ColorRole, 14> colorRoles
{
    QPalette::Window,
    QPalette::WindowText,
    QPalette::Button,
    QPalette::ButtonText,
    QPalette::BrightText,
    QPalette::Base,
    QPalette::AlternateBase,
    QPalette::Text,
    QPalette::ToolTipBase,
    QPalette::ToolTipText,
    QPalette::Highlight,
    QPalette::HighlightedText,
    QPalette::Link,
    QPalette::LinkVisited,
    // QPalette::Light,
    // QPalette::Midlight,
    // QPalette::Dark,
    // QPalette::Mid,
    // QPalette::Shadow
};


static void writePalette(const QPalette &palette, QSettings &s)
{
    if (!s.isWritable())
        throw runtime_error("Path is not writable.");

    for (auto group : colorGroups)
    {
        s.beginGroup(toString(group));
        for (auto role : colorRoles)
            s.setValue(toString(role), toString(palette.color(group, role)));
        s.endGroup();
    }
}

static QPalette readPalette(QSettings &s)
{
    auto getColor = [&s](QPalette::ColorRole role){
        if (auto color = s.value(toString(role)).value<QColor>();
            color.isValid())
            return color;
        else
            throw runtime_error(string("Invalid palette. Not found: ") + toString(role));
    };


    // QPalette(const QBrush &windowText,
    //          const QBrush &button,
    //          const QBrush &light,
    //          const QBrush &dark,
    //          const QBrush &mid,
    //          const QBrush &text,
    //          const QBrush &bright_text,
    //          const QBrush &base,
    //          const QBrush &window);

    s.beginGroup(toString(QPalette::Active));

    auto Base = getColor(QPalette::Base);
    auto Text = getColor(QPalette::Text);

    auto Window = getColor(QPalette::Window);
    auto WindowText = getColor(QPalette::WindowText);

    auto Button = getColor(QPalette::Button);
    auto ButtonText = getColor(QPalette::ButtonText);

    QColor Light, Mid, Dark;
    try {
        Light = getColor(QPalette::Light);
        Mid = getColor(QPalette::Mid);
        Dark = getColor(QPalette::Dark);
    } catch (...) {
        Light = Button.lighter();
        Mid = Button.darker();
        Dark = Mid.darker();
    }

    QPalette palette(WindowText,
                     Button,
                     Light,
                     Mid,
                     Dark,
                     Text,
                     ButtonText,
                     Base,
                     Window);

    // Mandatory, throws
    palette.setColor(QPalette::All, QPalette::Highlight, getColor(QPalette::Highlight));

    if (auto color = s.value(toString(QPalette::PlaceholderText)).value<QColor>();
        color.isValid())
        palette.setColor(QPalette::All, QPalette::PlaceholderText, color);
    else
        palette.setColor(QPalette::All, QPalette::PlaceholderText, Button);

    if (auto color = s.value(toString(QPalette::HighlightedText)).value<QColor>();
        color.isValid())
        palette.setColor(QPalette::All, QPalette::HighlightedText, color);
    else
        palette.setColor(QPalette::All, QPalette::HighlightedText, ButtonText);

    if (auto color = s.value(toString(QPalette::Link)).value<QColor>();
        color.isValid())
        palette.setColor(QPalette::All, QPalette::Link, color);

    if (auto color = s.value(toString(QPalette::LinkVisited)).value<QColor>();
        color.isValid())
        palette.setColor(QPalette::All, QPalette::LinkVisited, color);


    // QPalette palette;
    // for (auto group : colorGroups)
    // {
    //     s.beginGroup(toString(group));
    //     for (auto role : colorRoles)
    //         if (auto color = s.value(toString(role)).value<QColor>();
    //             color.isValid())
    //             palette.setColor(group, role, color);
    //         else
    //             throw runtime_error("Invalid palette.");
    //     s.endGroup();
    // }

    s.endGroup();

    return palette;
}

Theme::Theme():
    Theme(QApplication::style()->standardPalette())
{}

Theme::Theme(const QPalette &palette):
    Theme(palette,
          QColor(0, 0, 0, 92),                       // window_shadow_brush
          palette.brush(QPalette::Window),           // window_background_brush
          palette.brush(QPalette::Highlight),        // window_border_brush
          palette.brush(QPalette::Base),             // input_background_brush
          palette.brush(QPalette::Highlight),        // input_border_brush
          palette.color(QPalette::Button),           // settings_button_color
          palette.color(QPalette::Highlight),        // settings_button_highlight_color
          palette.brush(QPalette::Highlight),        // result_item_selection_background_brush
          palette.brush(QPalette::Highlight),        // result_item_selection_border_brush
          palette.color(QPalette::HighlightedText),  // result_item_selection_text_color
          palette.color(QPalette::PlaceholderText),  // result_item_selection_subtext_color
          palette.color(QPalette::WindowText),       // result_item_text_color
          palette.color(QPalette::PlaceholderText),  // result_item_subtext_color
          palette.brush(QPalette::Highlight),        // action_item_selection_background_brush
          palette.brush(QPalette::Highlight),        // action_item_selection_border_brush
          palette.color(QPalette::HighlightedText),  // action_item_selection_text_color
          palette.color(QPalette::WindowText))       // action_item_text_color
{

}

Theme::Theme(const QPalette &_palette,
             const QBrush &_window_shadow_brush,
             const QBrush &_window_background_brush,
             const QBrush &_window_border_brush,
             const QBrush &_input_background_brush,
             const QBrush &_input_border_brush,
             const QColor &_settings_button_color,
             const QColor &_settings_button_highlight_color,
             const QBrush &_result_item_selection_background_brush,
             const QBrush &_result_item_selection_border_brush,
             const QColor &_result_item_selection_text_color,
             const QColor &_result_item_selection_subtext_color,
             const QColor &_result_item_text_color,
             const QColor &_result_item_subtext_color,
             const QBrush &_action_item_selection_background_brush,
             const QBrush &_action_item_selection_border_brush,
             const QColor &_action_item_selection_text_color,
             const QColor &_action_item_text_color):
    palette(_palette),
    window_shadow_brush(_window_shadow_brush),
    window_background_brush(_window_background_brush),
    window_border_brush(_window_border_brush),
    input_background_brush(_input_background_brush),
    input_border_brush(_input_border_brush),
    settings_button_color(_settings_button_color),
    settings_button_highlight_color(_settings_button_highlight_color),
    result_item_selection_background_brush(_result_item_selection_background_brush),
    result_item_selection_border_brush(_result_item_selection_border_brush),
    result_item_selection_text_color(_result_item_selection_text_color),
    result_item_selection_subtext_color(_result_item_selection_subtext_color),
    result_item_text_color(_result_item_text_color),
    result_item_subtext_color(_result_item_subtext_color),
    action_item_selection_background_brush(_action_item_selection_background_brush),
    action_item_selection_border_brush(_action_item_selection_border_brush),
    action_item_selection_text_color(_action_item_selection_text_color),
    action_item_text_color(_action_item_text_color)
{}

Theme Theme::read(const QString &path)
{
    QSettings s(path, QSettings::IniFormat);

    auto palette = readPalette(s);

    auto t = Theme(palette);

#define readColor(val) t.val = s.value(QStringLiteral(#val), t.val).value<QColor>();\
    CRIT << s.group() << QStringLiteral(#val) << toString(t.val);
#define readBrush(val) t.val = s.value(QStringLiteral(#val), t.val).value<QBrush>();

    s.beginGroup(QStringLiteral("Window"));
    t.window_shadow_brush = s.value(keys.window_shadow_brush,
                                    t.window_shadow_brush).value<QColor>();

    readBrush(window_shadow_brush)
    readBrush(window_background_brush)
    readBrush(window_border_brush)
    readBrush(input_background_brush)
    readBrush(input_border_brush)
    readColor(settings_button_color)
    readColor(settings_button_highlight_color)
    readBrush(result_item_selection_background_brush)
    readBrush(result_item_selection_border_brush)
    readColor(result_item_selection_text_color)
    readColor(result_item_selection_subtext_color)
    readColor(result_item_text_color)
    readColor(result_item_subtext_color)
    readBrush(action_item_selection_background_brush)
    readBrush(action_item_selection_border_brush)
    readColor(action_item_selection_text_color)
    readColor(action_item_text_color)
    s.endGroup();

    return t;
}

void Theme::write(const Theme &theme, const QString &path)
{
    QSettings s(path, QSettings::IniFormat);

    if (!s.isWritable()){
        WARN << "Path not writable:" << path;
        return;
    }

    writePalette(theme.palette, s);

#define writeValue(val) s.setValue(QStringLiteral(#val), toString(theme.val));

    s.beginGroup(QStringLiteral("Window"));
    writeValue(window_shadow_brush)
    writeValue(window_background_brush)
    writeValue(window_border_brush)
    writeValue(input_background_brush)
    writeValue(input_border_brush)
    writeValue(settings_button_color)
    writeValue(settings_button_highlight_color)
    writeValue(result_item_selection_background_brush)
    writeValue(result_item_selection_border_brush)
    writeValue(result_item_selection_text_color)
    writeValue(result_item_selection_subtext_color)
    writeValue(result_item_text_color)
    writeValue(result_item_subtext_color)
    writeValue(action_item_selection_background_brush)
    writeValue(action_item_selection_border_brush)
    writeValue(action_item_selection_text_color)
    writeValue(action_item_text_color)
    s.endGroup();
}

void Theme::write(const QString &path) const { write(*this, path); }
