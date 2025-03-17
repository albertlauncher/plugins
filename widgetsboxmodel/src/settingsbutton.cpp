// Copyright (c) 2022-2025 Manuel Schneider

#include "settingsbutton.h"
#include "util.h"
#include <QMouseEvent>
#include <QPaintEvent>
#include <QPainter>
#include <QPixmapCache>
#include <QTimer>
#include <QtSvg/QSvgRenderer>
#include <albert/logging.h>
using namespace std;


SettingsButton::SettingsButton(QWidget *parent):
    QFrame(parent),
    color(Qt::transparent),
    rps(0)
{
    // once used animations but like 10% cpu even without drawing
    animation_timer_.setInterval(16); // ~60 fps, totally sufficient
    connect(&animation_timer_, &QTimer::timeout, this, [this]{
        auto degree_per_timer_interval = rps * 360 / 1000 * animation_timer_.interval();
        angle_ = std::fmod(angle_ + degree_per_timer_interval, 60);  // gear has 6 homomorphic permutations
        update();
    });

    svg_renderer_ = std::make_unique<QSvgRenderer>(QString(":gear"));

    setCursor(Qt::PointingHandCursor);

    setMinimumSize(0,0);
}

SettingsButton::~SettingsButton() = default;

bool SettingsButton::event(QEvent *event)
{
    if (event->type() == QEvent::Show)
        animation_timer_.start();

    else if (event->type() == QEvent::Hide)
        animation_timer_.stop();

    else if (event->type() == QEvent::MouseButtonPress)
    {
        emit clicked(static_cast<QMouseEvent*>(event)->button());
        return true;
    }

    return QWidget::event(event);
}

void SettingsButton::paintEvent(QPaintEvent *)
{
    auto pad = 0;//  rect().height() / 10;
    auto gear_rect = contentsRect().adjusted(pad,pad,-pad,-pad);

    QPixmap pm = QPixmap(gear_rect.size() * devicePixelRatioF());
    pm.fill(Qt::transparent);

    QPainter pp(&pm);
    QRectF pixmap_rect{{}, pm.size()};

    QPointF rotationOrigin = pixmap_rect.center();
    pp.translate(rotationOrigin);
    pp.rotate(angle_);
    pp.translate(-rotationOrigin);
    svg_renderer_->render(&pp);
    pp.resetTransform();
    pp.setCompositionMode(QPainter::CompositionMode_SourceIn);
    pp.fillRect(pixmap_rect, color);
    pm.setDevicePixelRatio(devicePixelRatioF());

    QPainter p(this);
    p.drawPixmap(gear_rect, pm);
}
