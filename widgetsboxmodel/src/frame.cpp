// Copyright (c) 2023-2025 Manuel Schneider

#include "frame.h"
#include "primitives.h"
#include <QPaintEvent>

Frame::Frame(QWidget *parent):
    QWidget(parent),
    border_brush_(Qt::transparent),
    fill_brush_(palette().color(QPalette::Window)),
    border_width_(0),
    radius_(0)
{
    setMinimumSize(0,0);
}

double Frame::borderWidth() const
{
    return border_width_;
}

void Frame::setBorderWidth(double boarderWidth)
{
    if (border_width_ == boarderWidth)
        return;

    border_width_ = boarderWidth;
    update();
    emit borderWidthChanged(border_width_);
}

double Frame::radius() const
{
    return radius_;
}

void Frame::setRadius(double radius)
{
    if (radius_ == radius)
        return;

    radius_ = radius;
    update();
    emit radiusChanged(radius_);
}

QBrush Frame::fillBrush() const
{
    return fill_brush_;
}

void Frame::setFillBrush(QBrush fillBrush)
{
    if (fill_brush_ == fillBrush)
        return;

    fill_brush_ = fillBrush;
    update();
    emit fillBrushChanged(fill_brush_);
}

QBrush Frame::borderBrush() const
{
    return border_brush_;
}

void Frame::setBorderBrush(QBrush boarderBrush)
{
    if (border_brush_ == boarderBrush)
        return;

    border_brush_ = boarderBrush;
    update();
    emit borderBrushChanged(border_brush_);
}

void Frame::paintEvent(QPaintEvent*)
{
    auto dpr = devicePixelRatioF();
    QPixmap pm = pixelPerfectRoundedRect(size() * dpr,
                                         fill_brush_,
                                         (int)(radius_ * dpr),
                                         border_brush_,
                                         (int)(border_width_ * dpr));
    pm.setDevicePixelRatio(dpr);

    QPainter p(this);
    p.drawPixmap(rect(), pm);
}

QSize Frame::minimumSizeHint() const { return {-1, -1}; }
