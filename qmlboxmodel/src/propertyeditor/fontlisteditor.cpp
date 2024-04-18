// Copyright (c) 2022-2024 Manuel Schneider

#include "fontlisteditor.h"
#include <QFontDatabase>
#include <QStandardItemModel>
#include <QTimer>

FontListEditor::FontListEditor(QWidget *parent):
    QComboBox(parent)
{
    populateList();
    QTimer::singleShot(0, this, &QComboBox::showPopup);
}

QFont FontListEditor::font() const
{ return currentText(); }

void FontListEditor::setFont(const QFont &font)
{ setCurrentText(font.family()); }

void FontListEditor::populateList()
{
    auto *model = new QStandardItemModel(this);
    for (const auto &font_family : QFontDatabase::families()){
        auto *item = new QStandardItem();
        item->setText(font_family);
        item->setData(font_family, Qt::FontRole);
        model->appendRow(item);
    }
    setModel(model);
}
