// Copyright (c) 2023-2025 Manuel Schneider

#pragma once
class QWidget;
class QStyle;
template<typename T> class QList;

bool haveDarkSystemPalette();

void setStyleRecursive(QWidget *widget, QStyle *style);

QList<QWidget*> getParents(QWidget* widget);
