// Copyright (c) 2024 Manuel Schneider

#include "inputframe.h"
#include "primitives.h"
#include "style.h"
#include <QLayout>
#include <QPaintEvent>
#include <QPixmapCache>
#include <QFontMetrics>

void InputFrame::setStyle(const Style *s)
{
    style_ = s;

    auto p = s->input_frame_padding + s->input_frame_border_width;

    // Fix for nicely aligned text.
    // The location of this code is  hacky, but QTextEdit does not allow to set margins.
    // The text should be idented by the distance of the cap line to the top.
    QFont f;
    f.setPointSize(s->input_line_font_size);
    QFontMetrics fm(f);
    auto font_margin_fix = (fm.lineSpacing() - fm.capHeight() - fm.tightBoundingRect("|").width())/2 ;

    // setContentsMargins(p, p, p, p);
    layout()->setContentsMargins(p + font_margin_fix, p, p, p);

    update();
}

void InputFrame::paintEvent(QPaintEvent *)
{
    QPixmap pm;
    if (const auto cache_key = QStringLiteral("_InputFrame_%1x%2")
                                   .arg(width()).arg(height());
        !QPixmapCache::find(cache_key, &pm))
    {
        auto dpr = devicePixelRatioF();
        pm = pixelPerfectRoundedRect(size() * dpr,
                                     style_->input_frame_background_brush,
                                     style_->input_frame_border_radius * dpr,
                                     style_->input_frame_border_brush,
                                     style_->input_frame_border_width * dpr);
        pm.setDevicePixelRatio(dpr);
        // QPixmapCache::insert(cache_key, pm);
    }

    QPainter p(this);
    p.drawPixmap(contentsRect(), pm);

    if (style_->draw_debug_overlays){
        drawDebugRect(p, rect(), "InputFrame::rect");
        drawDebugRect(p, rect(), "InputFrame::contents");
    }
}
