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
local output_path="$9"

echo "$(pwd)/${output_path}"

cat << EOF > "${output_path}"
/* Copyright ubervison <https://github.com/ubervison> */

* {
    border: none;
    color : ${foreground_color};
    background-color: ${background_color};
}

#frame {
    padding: 4px;
    border-radius: 3px;
    background-color: ${background_color};

    /* Workaround for Qt to get fixed size button*/
    min-width:640px;
    max-width:640px;
}

#inputLine {
    padding: 2px;
    border-radius: 2px;
    border: 1px solid ${border_color};
    font-size: 36px;
    selection-color: ${background_color};
    selection-background-color: ${foreground_color};
    background-color:  ${inputline_color};
}

#settingsButton {
    color : ${button_color};
    background-color: ${background_color};
    padding: 4px;

    /* Respect the frame border */
    margin: 6px 6px 0px 0px;
    border-top-right-radius: 6px;
    border-bottom-left-radius: 10px;

    /* Workaround for Qt to get fixed size button*/
    min-width:13px;
    min-height:13px;
    max-width:13px;
    max-height:13px;
}

/********** ListViews **********/

QListView {
    selection-color: ${foreground_color};
}

QListView::item:selected {
    background: ${selection_background_color};
    border: 1px solid ${selection_border_color};
}

QListView QScrollBar:vertical  {
    width: 5px;
    background: transparent;
}

QListView QScrollBar::handle:vertical  {
    background: ${scroll_color};
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

/********** actionList **********/

QListView#actionList {
    font-size: 20px;
}

QListView#actionList::item{
    height:28px;
}

/********** resultsList **********/

QListView#resultsList {
    icon-size: 44px;
    font-size: 26px;
}

QListView#resultsList::item{
    height:48px;
}
EOF
}

if [ "$#" -ne 1 ]; then
    echo "Usage: $0 <output_path>"
    exit 1
fi

#             bg        fg        input     border    button    scrollbar sel_bg    sel_border output
createStyle   "#e7e8eb" "#727A8F" "#fdfdfd" "#CFD6E6" "#ffffff" "#b8babf" "#95c4fb" "#cfd6e6" "$1/Arc Blue.qss"
createStyle   "#e7e8eb" "#727A8F" "#fdfdfd" "#CFD6E6" "#ffffff" "#b8babf" "#F5F6F7" "#4084D6" "$1/Arc Grey.qss"
createStyle   "#383C4A" "#AFB8C5" "#404552" "#21252B" "#ffffff" "#b8babf" "#4084D6" "#4084D6" "$1/Arc Dark Blue.qss"
createStyle   "#383C4A" "#AFB8C5" "#404552" "#21252B" "#ffffff" "#b8babf" "#404552" "#2B2E39" "$1/Arc Dark Grey.qss"
