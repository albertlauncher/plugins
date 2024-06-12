// Copyright (c) 2024 Manuel Schneider

#pragma once
#include "QtGui/qpalette.h"
#include <QApplication>
#include <QBrush>
#include <QFont>
#include <QMetaEnum>


template<typename QEnum>
static const char *qEnumToString(const QEnum value)
{ return QMetaEnum::fromType<QEnum>().valueToKey(value); }

QPalette loadPalette(const QString &path);

void savePalette(const QPalette &palette, const QString &path);


static std::vector<std::pair<const char*, QPalette::ColorRole>> mapping
{{
    { "Window", QPalette::Window },
    { "WindowText", QPalette::WindowText },
    { "Base", QPalette::Base },
    { "AlternateBase", QPalette::AlternateBase },
    { "Text", QPalette::Text },
    { "ToolTipBase", QPalette::ToolTipBase },
    { "ToolTipText", QPalette::ToolTipText },
    { "Button", QPalette::Button },
    { "ButtonText", QPalette::ButtonText },
    { "BrightText", QPalette::BrightText },
    { "Highlight", QPalette::Highlight },
    { "HighlightedText", QPalette::HighlightedText },
    { "Link", QPalette::Link },
    { "LinkVisited", QPalette::LinkVisited }
}};


class Style
{
public:
    bool draw_debug_overlays                           = false;

    QPalette palette                                   = QApplication::palette();

    QBrush window_background_brush                     = palette.window();
    QBrush window_border_brush                         = palette.mid();
    QBrush window_shadow_color                         = QColor(0, 0, 0, 64);  // palette.mid();
    int    window_shadow_size                          = 60;
    int    window_shadow_voffset                       = 10;
    int    window_border_radius                        = 16;  // item_padding * 2
    double window_border_width                         = 0.5;
    int    window_padding                              = 6;
    int    window_width                                = 640;

    QBrush input_frame_background_brush                = palette.base();
    QBrush input_frame_border_brush                    = palette.mid();
    int    input_frame_border_radius                   = 10;  // item_padding * 2
    int    input_frame_border_width                    = 0;  // item_padding * 2
    int    input_frame_padding                         = 0;

    int    input_line_font_size                        = QApplication::font().pointSize() + 9;

    QColor settings_button_color                       = palette.mid().color();
    QColor settings_button_highlight_color             = palette.highlight().color();
    int    settings_button_size                        = 16;

    QBrush item_view_background_brush                  = palette.window();
    QBrush item_view_border_brush                      = input_frame_border_brush;
    int    item_view_border_radius                     = input_frame_border_radius;
    int    item_view_border_width                      = 0;
    int    item_view_padding                           = 0;

    QBrush item_view_item_selection_background_brush   = palette.highlight();
    QBrush item_view_item_selection_border_brush       = palette.highlight().color().darker(120);
    int    item_view_item_selection_border_radius      = input_frame_border_radius;
    int    item_view_item_selection_border_width       = 0;
    int    item_view_item_padding                      = 6;

    QBrush result_item_text_color                      = palette.windowText();
    QBrush result_item_subtext_color                   = palette.placeholderText();
    int    result_item_icon_size                       = 30;
    int    result_item_text_font_size                  = QApplication::font().pointSize() + 3;
    int    result_item_subtext_font_size               = QApplication::font().pointSize() - 2;
    int    result_item_horizontal_spacing              = 6;
    int    result_item_vertical_spacing                = 1;

    QBrush action_item_text_color                      = palette.windowText();
    int    action_item_font_size                       = QApplication::font().pointSize() + 2;

public:

    static Style read(const QString &path);
    bool write(const QString &path) const;


    // static QColor parseColor(const QString&);

    static QString toString(const int &number);
    static QString toString(const QColor &color);
    static QString toString(const QBrush &brush);

};
