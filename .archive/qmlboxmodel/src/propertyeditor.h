// Copyright (c) 2022 Manuel Schneider

#pragma once
#include <QDialog>

namespace QmlBoxModel {

class MainWindow;

class PropertyEditor : public QDialog
{
    Q_OBJECT

public:
    explicit PropertyEditor(MainWindow *mainWindow, QWidget *parent = 0);
};

}
