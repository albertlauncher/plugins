// Copyright (c) 2023-2025 Manuel Schneider

#pragma once
#include <QPainter>



QPixmap pixelPerfectRoundedRect(QSize const &size,
                                QBrush const &fill_brush,
                                int const radius = 0,
                                QBrush const &border_brush = Qt::NoBrush,
                                int const border_width = 0);

void drawCheckerboard(QPainter &p, const QRect &rect, const QColor &dark, const QColor &light, const qreal width);

void drawEllipseBorder(QPainter &p, QRectF const &rect, QColor const &color, qreal const border_width);

void drawBrushMark(QPainter &p, QRect const &r, const QBrush &brush, const QColor &border_color, int border_width);

void drawDebugRect(QPainter &p, const QRectF &rect, const QString& name, const QColor &cornerColor = Qt::blue, const QColor &lineColor = QColor(128, 128, 128, 128));

void qt_blurImage(QPainter *p, QImage &blurImage, qreal radius, bool quality, bool alphaOnly, int transposed = 0);

QImage blurImage(QImage &src, qreal radius, bool quality, bool alphaOnly, int transposed = 0);
