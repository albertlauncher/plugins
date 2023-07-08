#!/bin/bash

TEMPLATE="StandardThemeTemplate.qss"

function makeStyle(){
  cat "${1}" | sed \
    -e "s/%background_color%/${2}/g" \
    -e "s/%foreground_color%/${3}/g" \
    -e "s/%inputline_color%/${4}/g" \
    -e "s/%border_color%/${5}/g" \
    -e "s/%button_color%/${6}/g" \
    -e "s/%scroll_color%/${7}/g" \
    -e "s/%selection_foreground_color%/${8}/g"\
    -e "s/%selection_background_color%/${9}/g" \
    -e "s/%selection_border_color%/${10}/g" > "${11}"
}

mkdir themes -p

#makeStyle            bg       fg       input    border   button   scroll   sel_fg   sel_bg   sel_bor  output
makeStyle "$TEMPLATE" "FFFFFF" "808080" "D0D0D0" "808080" "808080" "808080" "808080" "808080" "808080" "themes/Bright.qss"
makeStyle "$TEMPLATE" "FFFFFF" "808080" "D0D0D0" "FF8000" "FF8000" "FF8000" "808080" "FF8000" "FF8000" "themes/BrightOrange.qss"
makeStyle "$TEMPLATE" "FFFFFF" "808080" "D0D0D0" "FF0080" "FF0080" "FF0080" "808080" "FF0080" "FF0080" "themes/BrightMagenta.qss"
makeStyle "$TEMPLATE" "FFFFFF" "808080" "D0D0D0" "00FF80" "00FF80" "00FF80" "808080" "00FF80" "00FF80" "themes/BrightMint.qss"
makeStyle "$TEMPLATE" "FFFFFF" "808080" "D0D0D0" "80FF00" "80FF00" "80FF00" "808080" "80FF00" "80FF00" "themes/BrightGreen.qss"
makeStyle "$TEMPLATE" "FFFFFF" "808080" "D0D0D0" "0080FF" "0080FF" "0080FF" "808080" "0080FF" "0080FF" "themes/BrightBlue.qss"
makeStyle "$TEMPLATE" "FFFFFF" "808080" "D0D0D0" "8000FF" "8000FF" "8000FF" "808080" "8000FF" "8000FF" "themes/BrightViolet.qss"
makeStyle "$TEMPLATE" "404040" "808080" "202020" "808080" "808080" "808080" "808080" "808080" "808080" "themes/Dark.qss"
makeStyle "$TEMPLATE" "404040" "808080" "202020" "FF8000" "FF8000" "FF8000" "808080" "FF8000" "FF8000" "themes/DarkOrange.qss"
makeStyle "$TEMPLATE" "404040" "808080" "202020" "FF0080" "FF0080" "FF0080" "808080" "FF0080" "FF0080" "themes/DarkMagenta.qss"
makeStyle "$TEMPLATE" "404040" "808080" "202020" "00FF80" "00FF80" "00FF80" "808080" "00FF80" "00FF80" "themes/DarkMint.qss"
makeStyle "$TEMPLATE" "404040" "808080" "202020" "80FF00" "80FF00" "80FF00" "808080" "80FF00" "80FF00" "themes/DarkGreen.qss"
makeStyle "$TEMPLATE" "404040" "808080" "202020" "0080FF" "0080FF" "0080FF" "808080" "0080FF" "0080FF" "themes/DarkBlue.qss"
makeStyle "$TEMPLATE" "404040" "808080" "202020" "8000FF" "8000FF" "8000FF" "808080" "8000FF" "8000FF" "themes/DarkViolet.qss"
makeStyle "$TEMPLATE" "002b36" "93a1a1" "073642" "b58900" "b58900" "b58900" "93a1a1" "b58900" "b58900" "themes/SolarizedDarkYellow.qss"
makeStyle "$TEMPLATE" "002b36" "93a1a1" "073642" "cb4b16" "cb4b16" "cb4b16" "93a1a1" "cb4b16" "cb4b16" "themes/SolarizedDarkOrange.qss"
makeStyle "$TEMPLATE" "002b36" "93a1a1" "073642" "dc322f" "dc322f" "dc322f" "93a1a1" "dc322f" "dc322f" "themes/SolarizedDarkRed.qss"
makeStyle "$TEMPLATE" "002b36" "93a1a1" "073642" "d33682" "d33682" "d33682" "93a1a1" "d33682" "d33682" "themes/SolarizedDarkMagenta.qss"
makeStyle "$TEMPLATE" "002b36" "93a1a1" "073642" "6c71c4" "6c71c4" "6c71c4" "93a1a1" "6c71c4" "6c71c4" "themes/SolarizedDarkViolet.qss"
makeStyle "$TEMPLATE" "002b36" "93a1a1" "073642" "268bd2" "268bd2" "268bd2" "93a1a1" "268bd2" "268bd2" "themes/SolarizedDarkBlue.qss"
makeStyle "$TEMPLATE" "002b36" "93a1a1" "073642" "2aa198" "2aa198" "2aa198" "93a1a1" "2aa198" "2aa198" "themes/SolarizedDarkCyan.qss"
makeStyle "$TEMPLATE" "002b36" "93a1a1" "073642" "859900" "859900" "859900" "93a1a1" "859900" "859900" "themes/SolarizedDarkGreen.qss"
makeStyle "$TEMPLATE" "fdf6e3" "586e75" "eee8d5" "b58900" "b58900" "b58900" "586e75" "b58900" "b58900" "themes/SolarizedBrightYellow.qss"
makeStyle "$TEMPLATE" "fdf6e3" "586e75" "eee8d5" "cb4b16" "cb4b16" "cb4b16" "586e75" "cb4b16" "cb4b16" "themes/SolarizedBrightOrange.qss"
makeStyle "$TEMPLATE" "fdf6e3" "586e75" "eee8d5" "dc322f" "dc322f" "dc322f" "586e75" "dc322f" "dc322f" "themes/SolarizedBrightRed.qss"
makeStyle "$TEMPLATE" "fdf6e3" "586e75" "eee8d5" "d33682" "d33682" "d33682" "586e75" "d33682" "d33682" "themes/SolarizedBrightMagenta.qss"
makeStyle "$TEMPLATE" "fdf6e3" "586e75" "eee8d5" "6c71c4" "6c71c4" "6c71c4" "586e75" "6c71c4" "6c71c4" "themes/SolarizedBrightViolet.qss"
makeStyle "$TEMPLATE" "fdf6e3" "586e75" "eee8d5" "268bd2" "268bd2" "268bd2" "586e75" "268bd2" "268bd2" "themes/SolarizedBrightBlue.qss"
makeStyle "$TEMPLATE" "fdf6e3" "586e75" "eee8d5" "2aa198" "2aa198" "2aa198" "586e75" "2aa198" "2aa198" "themes/SolarizedBrightCyan.qss"
makeStyle "$TEMPLATE" "fdf6e3" "586e75" "eee8d5" "859900" "859900" "859900" "586e75" "859900" "859900" "themes/SolarizedBrightGreen.qss"
