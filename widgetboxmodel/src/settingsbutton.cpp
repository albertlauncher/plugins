// Copyright (C) 2014-2018 Manuel Schneider

#include "settingsbutton.h"
#include <QRect>
#include <QPainter>
#include <QTimer>
#include <QPaintEvent>
#include <QPropertyAnimation>
#include <QStyleOptionButton>
#include <QtSvg/QSvgRenderer>

/** ***************************************************************************/
WidgetBoxModel::SettingsButton::SettingsButton(QWidget *parent) : QPushButton(parent) {
    animation_ = new QPropertyAnimation(this, "angle");
    animation_->setDuration(10000);
    animation_->setStartValue(0);
    animation_->setEndValue(360);
    animation_->setLoopCount(-1);
    animation_->start();
    connect(animation_, &QPropertyAnimation::valueChanged, this, static_cast<void (QWidget::*)()>(&QWidget::update));

    svgRenderer_ = new QSvgRenderer(QString(":gear"));

    setCursor(Qt::PointingHandCursor);
}



/** ***************************************************************************/
WidgetBoxModel::SettingsButton::~SettingsButton() {
    delete animation_;
    delete svgRenderer_;
}



/** ***************************************************************************/
void WidgetBoxModel::SettingsButton::hideEvent(QHideEvent *event) {
    animation_->stop();
    QPushButton::hideEvent(event);
}



/** ***************************************************************************/
void WidgetBoxModel::SettingsButton::showEvent(QShowEvent *event) {
    animation_->start();
    QPushButton::showEvent(event);
}



/** ***************************************************************************/
void WidgetBoxModel::SettingsButton::paintEvent(QPaintEvent *event) {
    QPushButton::paintEvent(event);

    QStyleOptionButton option;
    option.initFrom(this);
    QRect contentRect = style()->subElementRect(QStyle::SE_PushButtonContents, &option, this);

    // Prepare image in pixmap using mask
#if QT_VERSION >= 0x050600  // TODO: Remove when 18.04 is released
    QPixmap gearPixmap(contentRect.size() * devicePixelRatioF());
#else
    QPixmap gearPixmap(contentRect.size());
#endif
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
