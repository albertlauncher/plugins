// Copyright (c) 2023-2025 Manuel Schneider

#pragma once
#include "frame.h"
#include <QWidget>

class WindowFrame : public Frame
{
    Q_OBJECT
    Q_PROPERTY(uint shadowSize READ shadowSize WRITE setShadowSize NOTIFY shadowSizeChanged FINAL)
    Q_PROPERTY(uint shadowOffset READ shadowOffset WRITE setShadowOffset NOTIFY shadowOffsetChanged FINAL)
    Q_PROPERTY(QBrush shadowBrush READ shadowBrush WRITE setShadowBrush NOTIFY shadowBrushChanged FINAL)

public:

    WindowFrame(QWidget *parent = nullptr);

    uint shadowSize() const;
    void setShadowSize(uint val);

    uint shadowOffset() const;
    void setShadowOffset(uint val);

    QBrush shadowBrush() const;
    void setShadowBrush(QBrush val);

protected:

    void paintEvent(QPaintEvent *event) override;
    QString cacheKey() const;
    void onPropertiesChanged();

    uint shadow_size_;
    uint shadow_offset_;
    QBrush shadow_brush_;

signals:

    void shadowSizeChanged(uint);
    void shadowOffsetChanged(uint);
    void shadowBrushChanged(QBrush);

};
