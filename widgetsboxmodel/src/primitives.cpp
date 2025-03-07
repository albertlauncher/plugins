// Copyright (c) 2024 Manuel Schneider

#include "primitives.h"
#include "QtWidgets/qtwidgetsexports.h"
#include <QPainterPath>

QPixmap pixelPerfectRoundedRect(QSize const &size, QBrush const &fill_brush, int const radius,
                                QBrush const &border_brush, int const border_width)
{
    QPixmap pixmap(size);
    pixmap.fill(Qt::transparent);
    QRect rect{pixmap.rect()};
    QPainter p(&pixmap);
    p.setRenderHint(QPainter::RenderHint::Antialiasing, true);
    p.setPen(Qt::NoPen);
    if (radius == 0)
    {
        if (border_width == 0 || border_brush == Qt::NoBrush)
        {
            p.setBrush(fill_brush);
            p.drawRect(rect);
        }
        else
        {
            p.setBrush(border_brush);
            p.drawRect(rect);
            const auto inner_rect = rect.marginsRemoved({border_width, border_width, border_width, border_width});
            p.setBrush(fill_brush);
            p.setCompositionMode(QPainter::CompositionMode_Source);
            p.drawRect(inner_rect);
        }
    }
    else
    {
        p.setRenderHint(QPainter::RenderHint::Antialiasing, true);
        if (border_width == 0 || border_brush == Qt::NoBrush)
        {
            p.setBrush(fill_brush);
            p.drawRoundedRect(rect, radius-border_width, radius-border_width);
        }
        else
        {
            p.setBrush(border_brush);
            p.drawRoundedRect(rect, radius, radius);
            const auto inner_rect = rect.marginsRemoved({border_width, border_width, border_width, border_width});
            p.setBrush(fill_brush);
            p.setCompositionMode(QPainter::CompositionMode_Source);
            p.drawRoundedRect(inner_rect, radius-border_width, radius-border_width);

        }
    }
    return pixmap;
}

void drawCheckerboard(QPainter &p, const QRect &rect, const QColor &dark, const QColor &light, const qreal width)
{
    const auto hCellCount = rect.width() / width;
    const auto vCellCount = rect.height() / width;

    p.setPen(Qt::NoPen);
    for (auto i = 0; i < hCellCount; ++i) {
        for (auto j = 0; j < vCellCount; ++j) {
            const auto &cellColor = (i + j) % 2 == 0 ? dark : light;
            const auto cellX = rect.x() + i * width;
            const auto cellY = rect.y() + j * width;
            const auto cellW = std::max(1., hCellCount - i) * width;
            const auto cellH = std::max(1., vCellCount - j) * width;
            const auto squareRect = QRectF(cellX, cellY, cellW, cellH);
            p.setBrush(cellColor);
            p.drawRect(squareRect);
        }
    }
}

void drawEllipseBorder(QPainter &p, QRectF const &rect, QColor const &color, qreal const border_width)
{
    const auto hbw = border_width / 2.;
    const auto border_rect = rect.marginsRemoved({ hbw, hbw, hbw, hbw });
    p.setPen(QPen(color, border_width, Qt::SolidLine));
    p.setBrush(Qt::NoBrush);
    p.drawEllipse(border_rect);
}

void drawBrushMark(QPainter &p, QRect const &r, const QBrush &brush, const QColor &border_color, int border_width)
{
    const auto dia = std::min({r.height(), r.width()});
    const auto rect = QRect(r.width()/2 - dia/2,
                            r.height()/2 - dia/2,
                            dia, dia);

    p.setRenderHint(QPainter::Antialiasing, true);

    if (!brush.isOpaque()) {
        p.save();
        QPainterPath clipPath;
        clipPath.addEllipse(rect.adjusted(1,1,-1,-1));
        p.setClipPath(clipPath);
        drawCheckerboard(p, rect, QColor(220, 220, 220), QColor(255, 255, 255), 8);
        p.restore();
    }

    // Fill
    p.setPen(Qt::NoPen);
    p.setBrush(brush);
    p.drawEllipse(rect);

    // Border
    border_width = std::max(1, border_width);
    drawEllipseBorder(p, rect, border_color, border_width);
}

void drawDebugRect(QPainter &p, const QRectF &rect, const QString& name, const QColor &cornerColor, const QColor &lineColor)
{
    p.save();

    p.setPen({lineColor, 1, Qt::DotLine});
    p.drawRect(rect);

    p.setPen({cornerColor, 1, Qt::SolidLine});
    p.drawLine(rect.topLeft(), rect.topLeft() + QPointF(5, 0));
    p.drawLine(rect.topLeft(), rect.topLeft() + QPointF(0, 5));
    p.drawLine(rect.topRight(), rect.topRight() + QPointF(-5, 0));
    p.drawLine(rect.topRight(), rect.topRight() + QPointF(0, 5));
    p.drawLine(rect.bottomLeft(), rect.bottomLeft() + QPointF(5, 0));
    p.drawLine(rect.bottomLeft(), rect.bottomLeft() + QPointF(0, -5));
    p.drawLine(rect.bottomRight(), rect.bottomRight() + QPointF(-5, 0));
    p.drawLine(rect.bottomRight(), rect.bottomRight() + QPointF(0, -5));

    QFont f;
    f.setPixelSize(8);
    p.setFont(f);
    p.drawText(rect.bottomLeft(), name);

    p.restore();
}

QT_BEGIN_NAMESPACE
extern Q_WIDGETS_EXPORT void qt_blurImage(QPainter *p, QImage &blurImage, qreal radius, bool quality, bool alphaOnly, int transposed/* = 0*/);
QT_END_NAMESPACE

QImage blurImage(QImage &src, qreal radius, bool quality, bool alphaOnly, int)
{
    QImage blurred(src.size(), QImage::Format_ARGB32_Premultiplied);
    blurred.fill(0);
    blurred.setDevicePixelRatio(src.devicePixelRatio());
    QPainter p(&blurred);
    qt_blurImage(&p, src, radius, quality, alphaOnly);
    p.end();
    return blurred;
}
