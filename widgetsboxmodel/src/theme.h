// Copyright (c) 2024-2025 Manuel Schneider

#pragma once
#include <QColor>
#include <QBrush>
#include <QPalette>
class QPalette;


class Theme
{
public:

    Theme();
    Theme(const QPalette &palette);

    static Theme read(const QString &path);

    QPalette palette;

    QBrush window_shadow_brush;
    QBrush window_background_brush;
    QBrush window_border_brush;
    QBrush input_background_brush;
    QBrush input_border_brush;
    QColor input_trigger_color;
    QColor input_hint_color;
    QColor settings_button_color;
    QColor settings_button_highlight_color;
    QBrush result_item_selection_background_brush;
    QBrush result_item_selection_border_brush;
    QColor result_item_selection_text_color;
    QColor result_item_selection_subtext_color;
    QColor result_item_text_color;
    QColor result_item_subtext_color;
    QBrush action_item_selection_background_brush;
    QBrush action_item_selection_border_brush;
    QColor action_item_selection_text_color;
    QColor action_item_text_color;

};
