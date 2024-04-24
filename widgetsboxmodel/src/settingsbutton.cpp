// Copyright (c) 2022-2024 Manuel Schneider

#include "settingsbutton.h"
#include <QAction>
#include <QPaintEvent>
#include <QPainter>
#include <QPropertyAnimation>
#include <QStyleOptionButton>
#include <QtSvg/QSvgRenderer>
#include <albert/util.h>


SettingsButton::SettingsButton(QWidget *parent) : QPushButton(parent)
{
    rotation_animation = std::make_unique<QPropertyAnimation>(this, "angle");
    rotation_animation->setDuration(4000);
    rotation_animation->setStartValue(0);
    rotation_animation->setEndValue(360);
    rotation_animation->setLoopCount(-1);
    connect(rotation_animation.get(), &QPropertyAnimation::valueChanged,
            this, static_cast<void (QWidget::*)()>(&QWidget::update));

    svgRenderer_ = std::make_unique<QSvgRenderer>(QString(":gear"));

    setCursor(Qt::PointingHandCursor);

    auto *action = new QAction("Settings", this);
    action->setShortcuts({QKeySequence("Ctrl+,"), QKeySequence("Alt+,")});
    connect(action, &QAction::triggered, this, [](){ albert::showSettings(); });
    connect(this, &QPushButton::clicked, action, &QAction::trigger);
}

SettingsButton::~SettingsButton() = default;

bool SettingsButton::event(QEvent *event)
{
    if (event->type() == QEvent::Paint)
    {
        auto *paint_event = static_cast<QPaintEvent*>(event);
        QPushButton::paintEvent(paint_event);

        QStyleOptionButton option;
        option.initFrom(this);
        QRect contentRect = style()->subElementRect(QStyle::SE_PushButtonContents, &option, this);

        // Prepare image in pixmap using mask
        QPixmap gearPixmap(contentRect.size() * devicePixelRatioF());
        gearPixmap.fill(Qt::transparent);

        QPointF rotationOrigin = QRectF(QPoint(), gearPixmap.size()).center();

        QPainter pixmapPainter(&gearPixmap);
        pixmapPainter.translate(rotationOrigin);
        pixmapPainter.rotate(angle_);
        pixmapPainter.translate(-rotationOrigin);
        svgRenderer_->render(&pixmapPainter);
        pixmapPainter.resetTransform();
        pixmapPainter.setCompositionMode(QPainter::CompositionMode_SourceIn);
        pixmapPainter.fillRect(gearPixmap.rect(), option.palette.windowText().color());

        // Draw pixmap on button
        QPainter painter(this);
        painter.drawPixmap(contentRect, gearPixmap);
    }

    else if (event->type() == QEvent::Show)
        rotation_animation->start();

    else if (event->type() == QEvent::Hide)
        rotation_animation->stop();

    return QPushButton::event(event);
}

