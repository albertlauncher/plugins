// Copyright (c) 2022-2024 Manuel Schneider

#pragma once
#include <QAbstractAnimation>
#include <QTimer>
#include <QWidget>
#include <memory>
class QPropertyAnimation;
class QSvgRenderer;
class Style;

class SettingsButton final : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(QColor color MEMBER color_)
    Q_PROPERTY(int speed MEMBER speed_)

public:
    explicit SettingsButton(QWidget *parent = nullptr);
    ~SettingsButton();

    void setStyle(const Style*);

    enum State { Hidden, Visible, Highlight };
    void setState(State s);

private:

    void paintEvent(QPaintEvent *event) override;
    bool event(QEvent *event) override;

    const Style *style_;
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
