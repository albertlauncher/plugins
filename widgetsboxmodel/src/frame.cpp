// Copyright (c) 2024 Manuel Schneider

#include "frame.h"
#include "primitives.h"
// #include <QLayout>
#include <QPaintEvent>
// #include <QPixmapCache>
// #include <QFontMetrics>

Frame::Frame(QWidget *parent):
    QWidget(parent),
    border_brush_(Qt::transparent),
    fill_brush_(palette().color(QPalette::Window)),
    border_width_(0),
    radius_(0)
{

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

extern bool draw_debug_overlays;

void Frame::paintEvent(QPaintEvent*)
{
    auto dpr = devicePixelRatioF();
    QPixmap pm = pixelPerfectRoundedRect(size() * dpr,
                                         fill_brush_,
                                         radius_ * dpr,
                                         border_brush_,
                                         border_width_ * dpr);
    pm.setDevicePixelRatio(dpr);

    QPainter p(this);
    p.drawPixmap(contentsRect(), pm);

    if (draw_debug_overlays)
    {
        drawDebugRect(p, rect(), QString("%1::rect").arg(objectName()));
        drawDebugRect(p, contentsRect(), QString("%1::contentsRect").arg(objectName()));
    }
}


// void >Frame::setStyle(const Style *s)
// {
//     style_ = s;

//     auto p = s->input_frame_padding + s->input_frame_border_width;

//     // Fix for nicely aligned text.
//     // The location of this code is  hacky, but QTextEdit does not allow to set margins.
//     // The text should be idented by the distance of the cap line to the top.
//     QFont f;
//     f.setPointSize(s->input_line_font_size);
//     QFontMetrics fm(f);
//     auto font_margin_fix = (fm.lineSpacing() - fm.capHeight() - fm.tightBoundingRect("|").width())/2 ;

//     // setContentsMargins(p, p, p, p);
//     layout()->setContentsMargins(p + font_margin_fix, p, p, p);

//     update();
// }
