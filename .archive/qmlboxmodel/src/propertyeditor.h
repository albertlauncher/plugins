// Copyright (c) 2022-2023 Manuel Schneider

#pragma once
#include "albert/logging.h"
#include <QDialog>
#include <QFontDatabase>
#include <QComboBox>
#include <QColorDialog>
#include <QStandardItemModel>
#include <QTimer>
class Plugin;

class PropertyEditor : public QDialog
{
    Q_OBJECT

public:
    explicit PropertyEditor(Plugin *plugin, QWidget *parent = 0);
};


class ColorDialog : public QColorDialog
{
    Q_OBJECT
    Q_PROPERTY(QColor color READ color WRITE setColor NOTIFY colorChanged USER true)
public:
    ColorDialog(QWidget * parent = 0) : QColorDialog(parent){
        setOptions(QColorDialog::ShowAlphaChannel);
        connect(this, &QColorDialog::currentColorChanged, this, &ColorDialog::colorChanged);
    }

    QColor color(){ return currentColor(); }
    void setColor(const QColor& c){ setCurrentColor(c); }

signals:
    void colorChanged(const QColor &color);
};


class FontListEditor : public QComboBox
{
    Q_OBJECT
    Q_PROPERTY(QFont font READ font WRITE setFont NOTIFY fontChanged USER true)
public:
    FontListEditor(QWidget *parent = nullptr) : QComboBox(parent) {
        populateList();
        QTimer::singleShot(0, this, &QComboBox::showPopup);
    }

public:
    QFont font() const
    { return currentText(); }
    void setFont(const QFont &font)
    { setCurrentText(font.family()); }

private:
    void populateList()
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

signals:
    void fontChanged();
};
