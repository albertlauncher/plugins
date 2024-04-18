// Copyright (c) 2022-2024 Manuel Schneider

#include "colordialog.h"
// #include "propertyeditor.h"
// #include <QAbstractTableModel>
// #include <QHeaderView>
// #include <QItemEditorFactory>
// #include <QStyledItemDelegate>
// #include <QTableView>
// #include <QVBoxLayout>

ColorDialog::ColorDialog(QWidget *parent):
    QColorDialog(parent)
{
    setOptions(QColorDialog::ShowAlphaChannel);
    connect(this, &QColorDialog::currentColorChanged, this, &ColorDialog::colorChanged);
}

QColor ColorDialog::color()
{ return currentColor(); }

void ColorDialog::setColor(const QColor &c)
{ setCurrentColor(c); }
