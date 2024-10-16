// Copyright (c) 2022-2024 Manuel Schneider

#pragma once
#include <QDialog>

class PropertyEditor : public QDialog
{
    Q_OBJECT

public:
    explicit PropertyEditor(QObject *style_object, QWidget *parent = nullptr);
};

