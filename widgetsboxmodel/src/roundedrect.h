// Copyright (c) 2024 Manuel Schneider

#pragma once
#include <QWidget>

class RoundedRect : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(QBrush borderBrush READ borderBrush WRITE setBorderBrush NOTIFY borderBrushChanged FINAL)
    Q_PROPERTY(QBrush fillBrush READ fillBrush WRITE setFillBrush NOTIFY fillBrushChanged FINAL)
    Q_PROPERTY(double borderWidth READ borderWidth WRITE setBorderWidth NOTIFY borderWidthChanged FINAL)
    Q_PROPERTY(double radius READ radius WRITE setRadius NOTIFY radiusChanged FINAL)

public:

    RoundedRect(QWidget *parent = nullptr);

    double radius() const;
    void setRadius(double radius);

    QBrush fillBrush() const;
    void setFillBrush(QBrush fillBrush);

    double borderWidth() const;
    void setBorderWidth(double borderWidth);

    QBrush borderBrush() const;
    void setBorderBrush(QBrush borderBrush);


private:

    void paintEvent(QPaintEvent *event) override;

    QBrush border_brush_;
    QBrush fill_brush_;
    double border_width_;
    double radius_;


signals:

    void borderBrushChanged(const QBrush&);
    void borderWidthChanged(double);
    void fillBrushChanged(const QBrush&);
    void radiusChanged(double);

};
