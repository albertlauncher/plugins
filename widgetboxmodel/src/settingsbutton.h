// Copyright (C) 2014-2018 Manuel Schneider

#pragma once
#include <QPushButton>
class QPropertyAnimation;
class QSvgRenderer;

namespace WidgetBoxModel {

class SettingsButton final : public QPushButton
{
    Q_OBJECT
    Q_PROPERTY(int angle MEMBER angle_)

public:

    SettingsButton(QWidget *parent = 0);
    ~SettingsButton();

protected:

    void hideEvent(QHideEvent * event) override;
    void showEvent(QShowEvent * event) override;

private:

    void paintEvent(QPaintEvent *event) override;

    int angle_;
    QPropertyAnimation *animation_;
    QSvgRenderer *svgRenderer_;

};
}
