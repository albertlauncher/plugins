// Copyright (c) 2024 Manuel Schneider

#include "brushbutton.h"
#include "primitives.h"
#include <QColorDialog>
#include <QPainter>
#include <QStyle>

BrushButton::BrushButton(QWidget *parent):
    QAbstractButton(parent), brush_(Qt::NoBrush)
{

}

BrushButton::BrushButton(const QBrush &brush, QWidget *parent):
    QAbstractButton(parent), brush_(brush)
{
    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    // setFixedSize(20,20);

    QObject::connect(this, &QAbstractButton::clicked, this, [this]() {
        const auto newColor = QColorDialog::getColor(brush_.color(), this, {}, QColorDialog::ShowAlphaChannel);
        if (newColor.isValid()) {
            setBrush(newColor);
        }
    });
}

const QBrush &BrushButton::brush() const { return brush_; }

void BrushButton::setBrush(const QBrush &brush)
{
    if (brush_ != brush)
    {
        brush_ = brush;
        update();
        emit brushChanged();
    }
}

QSize BrushButton::sizeHint() const
{
    auto extent = this->style()->pixelMetric(QStyle::PM_DialogButtonsButtonHeight);
    return { extent, extent };
}

void BrushButton::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    auto r = contentsRect();
    drawBrushMark(p, r, brush_, QColor(0, 0, 0, 64), 2);
}
