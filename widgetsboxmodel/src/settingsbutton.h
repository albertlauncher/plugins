// Copyright (c) 2022-2024 Manuel Schneider

#pragma once
#include <QPushButton>
#include <memory>
class QPropertyAnimation;
class QSvgRenderer;

class SettingsButton final : public QPushButton
{
    Q_OBJECT
    Q_PROPERTY(int angle MEMBER angle_)

public:

    explicit SettingsButton(QWidget *parent = 0);
    ~SettingsButton() override;

private:

    bool event(QEvent *event) override;

    int angle_;
    std::unique_ptr<QSvgRenderer> svgRenderer_;
    std::unique_ptr<QPropertyAnimation> rotation_animation;
    QPixmap gearPixmap_;

};
