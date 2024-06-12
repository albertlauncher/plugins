// Copyright (c) 2024 Manuel Schneider

#pragma once
#include <QWidget>
class Style;

class InputFrame : public QWidget
{
public:
    using QWidget::QWidget;
    void setStyle(const Style*);

private:
    void paintEvent(QPaintEvent *event) override;
    const Style *style_;
};
