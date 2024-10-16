// Copyright (c) 2022-2024 Manuel Schneider

#pragma once
#include <QComboBox>

class FontListEditor : public QComboBox
{
    Q_OBJECT
    Q_PROPERTY(QFont font READ font WRITE setFont NOTIFY fontChanged USER true)

public:
    FontListEditor(QWidget *parent = nullptr);

public:
    QFont font() const;
    void setFont(const QFont &font);

private:
    void populateList();

signals:
    void fontChanged();
};
