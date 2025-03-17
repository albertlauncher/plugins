// Copyright (c) 2022-2025 Manuel Schneider

#pragma once
#include <QTimer>
#include <QFrame>
#include <memory>
class QSvgRenderer;

class SettingsButton final : public QFrame
{
    Q_OBJECT
    Q_PROPERTY(QColor color MEMBER color)
    Q_PROPERTY(double speed MEMBER rps)

public:

    explicit SettingsButton(QWidget *parent = nullptr);
    ~SettingsButton();

    QColor color;
    double rps;

private:

    void paintEvent(QPaintEvent *event) override;
    bool event(QEvent *event) override;

    std::unique_ptr<QSvgRenderer> svg_renderer_;
    QTimer animation_timer_;
    double angle_;

signals:

    void clicked(Qt::MouseButton);

};
