// Copyright (c) 2024 Manuel Schneider

#pragma once
#include <QObject>
class QPainter;

class DebugOverlay : public QObject
{
public:
    bool eventFilter(QObject *object, QEvent *event) override;

    void recursiveInstallEventFilter(QWidget *widget);

    void drawOverlay(QWidget *widget);
};
