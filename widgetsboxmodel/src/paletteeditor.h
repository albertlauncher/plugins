// Copyright (c) 2024-2025 Manuel Schneider

#pragma once
#include <QDialog>
class PaletteModel;

class PaletteEditor : public QDialog
{
    Q_OBJECT
    Q_PROPERTY(QPalette palette READ palette NOTIFY paletteChanged)

public:

    PaletteEditor(const QPalette &palette, QWidget *parent = nullptr);

    QPalette palette() const;

private:

    PaletteModel *model_;

signals:

    void paletteChanged(const QPalette &palette);
};
