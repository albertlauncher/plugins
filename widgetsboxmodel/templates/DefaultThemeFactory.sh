#!/bin/bash

function createStyle()
{
local background_color="$1"
local foreground_color="$2"
local button_color="$3"
local scroll_color="$4"
local selection_foreground_color="$5"
local selection_background_color="$6"
local output_path="$7"

local outer_padding=8
local radius=$((outer_padding + 2))
local inner_padding=2
#local outer_radius=$((border_size + inner_padding + outer_padding))

local input_font_size=24
local item_font_size=18
local action_font_size=14

local icon_size=36

echo "$(pwd)/${output_path}"
cat << EOF > "${output_path}"
/* Copyright: Manuel Schneider, License: MIT */

* {
    border: none;
    color : ${foreground_color};
    background-color: ${background_color};
}

#frame {
    border-radius: ${radius}px;
    background-color: ${background_color};
    min-width:640px;
    max-width:640px;
}

#inputLine {
    padding: ${outer_padding}px;
    font-size: ${input_font_size}px;
    selection-color: ${selection_foreground_color};
    selection-background-color: ${selection_background_color};
    background-color: transparent;
}

#settingsButton {
    margin: ${outer_padding}px;
    color: ${button_color};
    background-color: transparent;
    padding: 0px;
    min-width:14px;
    min-height:14px;
    max-width:14px;
    max-height:14px;
}

/********** ListViews ************/

QListView {
    margin-bottom: ${outer_padding}px;
    background: transparent;
    selection-color: ${selection_foreground_color};
}

QListView::item:selected {
    background: ${selection_background_color};
}

QListView QScrollBar:vertical {
    width: 3px;
    background: transparent;
}

QListView QScrollBar::handle:vertical  {
    background: ${scroll_color};
    border-radius: 1px;
    min-height: 24px;
}

QListView QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical,
QListView QScrollBar::up-arrow:vertical, QScrollBar::down-arrow:vertical,
QListView QScrollBar::add-page:vertical, QScrollBar::sub-page:vertical {
    border: 0px;
    width: 0px;
    height: 0px;
    background: transparent;
}

/********** resultsList **********/

QListView#resultsList {
    icon-size: ${icon_size}px;
    font-size: ${item_font_size}px;
    border-bottom-left-radius: ${radius}px;
    border-bottom-right-radius: ${radius}px;
}

QListView#resultsList::item {
    padding: 8px 0px 0px 0px;
    height: 48px;
}

/********** actionList **********/

QListView#actionList {
    font-size: ${action_font_size}px;
}

QListView#actionList::item {
    padding: 4px;
}
EOF
}

if [ "$#" -ne 1 ]; then
    echo "Usage: $0 <output_path>"
    exit 1
fi

# https://doc.qt.io/qt-6/stylesheet-reference.html#paletterole

#makeStyle  bg                fg                     button               scroll                 sel_fg                      sel_bg               output
createStyle "palette(window)" "palette(window-text)" "palette(highlight)" "palette(highlight)"   "palette(highlighted-text)" "palette(highlight)" "$1/Default System Palette.qss"
createStyle "#fcfcfc"         "#808080"              "#808080"            "#808080"              "#606060"                   "#e0e0e0"            "$1/Default Light.qss"
createStyle "#323232"         "#808080"              "#808080"            "#808080"              "#a0a0a0"                   "#505050"            "$1/Default Dark.qss"
