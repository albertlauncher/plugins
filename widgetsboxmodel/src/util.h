// Copyright (c) 2023-2025 Manuel Schneider

#pragma once
class QWidget;
class QStyle;

///
/// Hiding/Showing a window does not generate a Leave/Enter event. As such QWidget does not
/// update the internal underMouse property on show if the window is has been hidden and the
/// mouse pointer moved outside the widget.
///
bool isUnderMouse(QWidget *widget);

bool haveDarkSystemPalette();

void setStyleRecursive(QWidget *widget, QStyle *style);
