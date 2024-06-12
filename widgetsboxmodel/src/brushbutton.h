// Copyright (c) 2024 Manuel Schneider

#pragma once
#include <QAbstractButton>
#include <QBrush>

class BrushButton : public QAbstractButton
{
    Q_OBJECT
    Q_PROPERTY(QBrush brush READ brush WRITE setBrush NOTIFY brushChanged)

public:
    BrushButton(QWidget* parent = nullptr);
    BrushButton(const QBrush& brush, QWidget* parent = nullptr);

    const QBrush& brush() const;
    void setBrush(const QBrush& brush);

private:
    QSize sizeHint() const override;
    void paintEvent(QPaintEvent*) override;

    QBrush brush_;

signals:
    void brushChanged();
};

