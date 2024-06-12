// Copyright (c) 2024 Manuel Schneider

#include "util.h"
#include <QPalette>

bool haveDarkSystemPalette()
{
    const QPalette pal;
    return pal.color(QPalette::WindowText).lightness()
           > pal.color(QPalette::Window).lightness();
}
