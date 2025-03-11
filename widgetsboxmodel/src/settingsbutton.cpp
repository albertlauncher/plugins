// Copyright (c) 2022-2024 Manuel Schneider

#include "settingsbutton.h"
#include <QMouseEvent>
#include <QPaintEvent>
#include <QPainter>
#include <QParallelAnimationGroup>
#include <QPixmapCache>
#include <QPropertyAnimation>
#include <QSequentialAnimationGroup>
#include <QStyleOptionButton>
#include <QTimer>
#include <QtSvg/QSvgRenderer>
#include <albert/logging.h>
using namespace std;

SettingsButton::SettingsButton(QWidget *parent) : QFrame(parent)
{
    // set once initially otherwise the initial anim starts from black
    color_ = Qt::transparent;
    speed_ = 1;

    // once used animations but like 10% cpu even without drawing
    animation_timer_.setInterval(33); // ~30 fps, totally sufficient
    connect(&animation_timer_, &QTimer::timeout, this, [this]{
        angle_ += speed_;
        angle_ %= 60;  // gear has 6 homomorphic permutations
        update();
    });

    svgRenderer_ = std::make_unique<QSvgRenderer>(QString(":gear"));

    setCursor(Qt::PointingHandCursor);

    setMinimumSize(0,0);
}

SettingsButton::~SettingsButton() = default;

void SettingsButton::setState(State s)
{
    switch (s) {
    case Hidden:{
        auto a = make_unique<QPropertyAnimation>(this, "color");
        auto c = color.visible;
        c.setAlpha(0);
        a->setEndValue(c);
        connect(a.get(), &QAbstractAnimation::finished, this, &SettingsButton::hide);
        animation_ = ::move(a);
        break;
    }

    case Visible:{
        show();
        auto a = make_unique<QPropertyAnimation>(this, "color");
        a->setEndValue(color.visible);
        connect(a.get(), &QAbstractAnimation::finished, this, [this]{ speed_ = 2; });
        animation_ = ::move(a);
        break;
    }

    case Highlight:{
        show();
        speed_ = 3;
        auto a = make_unique<QPropertyAnimation>(this, "color");
        // if qt versio higher 6.0.0
        a->setEndValue(color.highlight);
        animation_ = ::move(a);
        break;
    }
    }
    animation_->start();
}

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
    // DEBG << contentsRect() << minimumSize() << rect();

    QPixmap pm = QPixmap(gear_rect.size() * devicePixelRatioF());
    pm.fill(Qt::transparent);

    QPainter pp(&pm);
    QRectF pixmap_rect{{}, pm.size()};

    QPointF rotationOrigin = pixmap_rect.center();
    pp.translate(rotationOrigin);
    pp.rotate(angle_);
    pp.translate(-rotationOrigin);
    svgRenderer_->render(&pp);
    pp.resetTransform();
    pp.setCompositionMode(QPainter::CompositionMode_SourceIn);
    pp.fillRect(pixmap_rect, color_);
    pm.setDevicePixelRatio(devicePixelRatioF());

    QPainter p(this);
    p.drawPixmap(gear_rect, pm);
}
