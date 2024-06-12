#!/bin/bash

function createStyle()
{
local background_color="$1"
local foreground_color="$2"
local inputline_color="$3"
local border_color="$4"
local button_color="$5"
local scroll_color="$6"
local selection_foreground_color="$7"
local selection_background_color="$8"
local selection_border_color="$9"
local output_path="${10}"

local border_size=0
local outer_padding=6
local inner_padding=4
local inner_radius=$((inner_padding * 2 + 2))
local outer_radius=$((border_size + inner_radius + outer_padding))

local input_font_size=24
local item_font_size=18
local action_font_size=14

local icon_size=36


echo "$(pwd)/${output_path}"
#tee "${output_path}" << EOF
cat << EOF > "${output_path}"
/* Copyright: Manuel Schneider, License: MIT */

* {
    border: none;
    color : ${foreground_color};
    background-color: ${background_color};
}

#frame {
    padding: ${outer_padding}px;
    border-radius: ${outer_radius}px;
    background-color: ${background_color};
    border: ${border_size}px solid ${border_color};
    min-width: 640px;
    max-width: 640px;
}

#inputLine {
    padding: ${inner_padding}px;
    border-radius: ${inner_radius}px;
    font-size: ${input_font_size}px;
    selection-color: ${background_color};
    selection-background-color: ${foreground_color};
    background-color: ${inputline_color};
}

#settingsButton {
    color: red; /*${button_color};*/
    background-color: blue; /*${background_color};*/
    padding: ${outer_padding}px;
    /* Respect the frame border */
    margin: ${border_size}px ${border_size}px 0px 0px;
    border-top-right-radius: ${outer_radius}px;
    border-bottom-left-radius: ${outer_radius}px;
    /* Workaround for Qt to get fixed size button*/
    min-width:  $((input_font_size/5*3))px;
    min-height: $((input_font_size/5*3))px;
    max-width:  $((input_font_size/5*3))px;
    max-height: $((input_font_size/5*3))px;
}

/********** ListViews **********/

QListView {
    margin-top: ${outer_padding}px;
    background: transparent;
    selection-color: ${selection_foreground_color};
}

QListView::item {
    border-radius: ${inner_radius}px;
}

QListView::item:selected {
    background: ${selection_background_color};
}

QListView QScrollBar:vertical {
    width: 2px;
    background: transparent;
}

QListView QScrollBar::handle:vertical  {
    background: ${scroll_color};
    min-height: 20px;
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
}

QListView#resultsList::item {
    /*margin: ${inner_padding}px 0px ${inner_padding}px 0px;*/
    height: 44px;
    padding: ${inner_padding}px;
}

/********** actionList **********/

QListView#actionList {
    font-size: ${action_font_size}px;
}


QListView#actionList::item{
    padding: ${inner_padding}px;
}

EOF
}


if [ "$#" -ne 1 ]; then
    echo "Usage: $0 <output_path>"
    exit 1
fi


#           bg                fg                     input           border               button
createStyle "palette(window)" "palette(window-text)" "palette(base)" "palette(highlight)" "palette(highlight)" \
            "palette(highlight)" "palette(HighlightedText)" "palette(highlight)"  "palette(window-text)" "$1/Legacy System Palette.qss"
#           scroll               sel_fg                     sel_bg                sel_bor                output


#           bg        fg        input     border    button    scroll    sel_fg    sel_bg    sel_bor   output
createStyle "#FFFFFF" "#808080" "#D0D0D0" "#808080" "#808080" "#808080" "#808080" "#D0D0D0" "#808080" "$1/Legacy Bright.qss"
createStyle "#FFFFFF" "#808080" "#D0D0D0" "#FF8000" "#FF8000" "#FF8000" "#FF8000" "#D0D0D0" "#FF8000" "$1/Legacy Bright Orange.qss"
createStyle "#FFFFFF" "#808080" "#D0D0D0" "#FF0080" "#FF0080" "#FF0080" "#FF0080" "#D0D0D0" "#FF0080" "$1/Legacy Bright Magenta.qss"
createStyle "#FFFFFF" "#808080" "#D0D0D0" "#00FF80" "#00FF80" "#00FF80" "#00FF80" "#D0D0D0" "#00FF80" "$1/Legacy Bright Mint.qss"
createStyle "#FFFFFF" "#808080" "#D0D0D0" "#80FF00" "#80FF00" "#80FF00" "#80FF00" "#D0D0D0" "#80FF00" "$1/Legacy Bright Green.qss"
createStyle "#FFFFFF" "#808080" "#D0D0D0" "#0080FF" "#0080FF" "#0080FF" "#0080FF" "#D0D0D0" "#0080FF" "$1/Legacy Bright Blue.qss"
createStyle "#FFFFFF" "#808080" "#D0D0D0" "#8000FF" "#8000FF" "#8000FF" "#8000FF" "#D0D0D0" "#8000FF" "$1/Legacy Bright Violet.qss"

createStyle "#404040" "#808080" "#202020" "#808080" "#808080" "#808080" "#808080" "#202020" "#808080" "$1/Legacy Dark.qss"
createStyle "#404040" "#808080" "#202020" "#FF8000" "#FF8000" "#FF8000" "#FF8000" "#202020" "#FF8000" "$1/Legacy Dark Orange.qss"
createStyle "#404040" "#808080" "#202020" "#FF0080" "#FF0080" "#FF0080" "#FF0080" "#202020" "#FF0080" "$1/Legacy Dark Magenta.qss"
createStyle "#404040" "#808080" "#202020" "#00FF80" "#00FF80" "#00FF80" "#00FF80" "#202020" "#00FF80" "$1/Legacy Dark Mint.qss"
createStyle "#404040" "#808080" "#202020" "#80FF00" "#80FF00" "#80FF00" "#80FF00" "#202020" "#80FF00" "$1/Legacy Dark Green.qss"
createStyle "#404040" "#808080" "#202020" "#0080FF" "#0080FF" "#0080FF" "#0080FF" "#202020" "#0080FF" "$1/Legacy Dark Blue.qss"
createStyle "#404040" "#808080" "#202020" "#8000FF" "#8000FF" "#8000FF" "#8000FF" "#202020" "#8000FF" "$1/Legacy Dark Violet.qss"


#https://en.wikipedia.org/wiki/Solarized

base03="#002b36"
base02="#073642"
base01="#586e75"
base00="#657b83"
base0="#839496"
base1="#93a1a1"
base2="#eee8d5"
base3="#fdf6e3"

Yellow="#b58900"
Orange="#cb4b16"
Red="#dc322f"
Magenta="#d33682"
Violet="#6c71c4"
Blue="#268bd2"
Cyan="#2aa198"
Green="#859900"

function createBrightSolarizedStyle()
{
    #           bg       fg       input    border button scroll sel_fg sel_bg   sel_bor output
    createStyle "$base3" "$base0" "$base2" "$1"   "$1"   "$1"   "$1"   "$base2" "$1"    "$2"
}

function createDarkSolarizedStyle()
{
    #           bg        fg        input     border button scroll sel_fg sel_bg    sel_bor output
    createStyle "$base03" "$base00" "$base02" "$1"   "$1"   "$1"   "$1"   "$base02" "$1"    "$2"
}

createBrightSolarizedStyle "$Yellow"  "$1/Legacy Solarized Bright Yellow.qss"
createBrightSolarizedStyle "$Orange"  "$1/Legacy Solarized Bright Orange.qss"
createBrightSolarizedStyle "$Red"     "$1/Legacy Solarized Bright Red.qss"
createBrightSolarizedStyle "$Magenta" "$1/Legacy Solarized Bright Magenta.qss"
createBrightSolarizedStyle "$Violet"  "$1/Legacy Solarized Bright Violet.qss"
createBrightSolarizedStyle "$Blue"    "$1/Legacy Solarized Bright Blue.qss"
createBrightSolarizedStyle "$Cyan"    "$1/Legacy Solarized Bright Cyan.qss"
createBrightSolarizedStyle "$Green"   "$1/Legacy Solarized Bright Green.qss"

createDarkSolarizedStyle "$Yellow"  "$1/Legacy Solarized Dark Yellow.qss"
createDarkSolarizedStyle "$Orange"  "$1/Legacy Solarized Dark Orange.qss"
createDarkSolarizedStyle "$Red"     "$1/Legacy Solarized Dark Red.qss"
createDarkSolarizedStyle "$Magenta" "$1/Legacy Solarized Dark Magenta.qss"
createDarkSolarizedStyle "$Violet"  "$1/Legacy Solarized Dark Violet.qss"
createDarkSolarizedStyle "$Blue"    "$1/Legacy Solarized Dark Blue.qss"
createDarkSolarizedStyle "$Cyan"    "$1/Legacy Solarized Dark Cyan.qss"
createDarkSolarizedStyle "$Green"   "$1/Legacy Solarized Dark Green.qss"

