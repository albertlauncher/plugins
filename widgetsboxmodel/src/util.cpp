// Copyright (c) 2023-2025 Manuel Schneider

#include "util.h"
#include <QWidget>
#include <QApplication>
#include <QStyle>

bool haveDarkSystemPalette()
{
    auto pal = QApplication::style()->standardPalette();
    return pal.color(QPalette::WindowText).lightness()
           > pal.color(QPalette::Window).lightness();
}

void setStyleRecursive(QWidget *widget, QStyle *style)
{
    widget->setStyle(style);
    for (auto child : widget->findChildren<QWidget*>())
        setStyleRecursive(child, style);
}

QList<QWidget *> getParents(QWidget *widget)
{
    QList<QWidget*> parents;
    while (widget->parentWidget())
    {
        widget = widget->parentWidget();
        parents.append(widget);
    }
    return parents;
}
