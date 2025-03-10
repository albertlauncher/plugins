// Copyright (c) 2022-2024 Manuel Schneider

#pragma once
#include <QAbstractAnimation>
#include <QTimer>
#include <QFrame>
#include <memory>
class QPropertyAnimation;
class QSvgRenderer;

class SettingsButton final : public QFrame
{
    Q_OBJECT
    Q_PROPERTY(QColor color MEMBER color_)
    Q_PROPERTY(int speed MEMBER speed_)

public:

    explicit SettingsButton(QWidget *parent = nullptr);
    ~SettingsButton();

    enum State { Hidden, Visible, Highlight };
    void setState(State state);

private:

    void paintEvent(QPaintEvent *event) override;
    bool event(QEvent *event) override;

    std::unique_ptr<QSvgRenderer> svgRenderer_;
    QTimer animation_timer_;
    QColor color_;
    int angle_;
    int speed_;
    QAction *action_;
    std::unique_ptr<QAbstractAnimation> animation_;

signals:
    void clicked(Qt::MouseButton);
};
