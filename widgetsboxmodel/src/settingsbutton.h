// Copyright (c) 2022 Manuel Schneider

#pragma once
#include <QPushButton>
#include <QPropertyAnimation>
class QSvgRenderer;

class SettingsButton final : public QPushButton
{
    Q_OBJECT
    Q_PROPERTY(int angle MEMBER angle_)

public:
    explicit SettingsButton(QWidget *parent = 0);
    ~SettingsButton() override;

    std::unique_ptr<QPropertyAnimation> rotation_animation;

private:
    void paintEvent(QPaintEvent *event) override;

    int angle_ = 0;
    std::unique_ptr<QSvgRenderer> svgRenderer_;
    QPixmap gearPixmap_;
};
