// Copyright (c) 2024 Manuel Schneider

#pragma once
#include <QWidget>
#include "style.h"
#include "ui_styleeditor.h"
class Window;

class StyleEditor : public QWidget
{
    Q_OBJECT

    StyleEditor(Window *window, const QString &styleFile = {});
    ~StyleEditor();


    void keyPressEvent(QKeyEvent *event) override;

    void initStylesGroupBox();

    void setStyleFile(const QString &path);

    void onChoosePaletteFromColor();
    void onChoosePaletteFromStyle();


    void onStyleCopy();
    void onStyleRemove();
    void onStyleImport();
    void onStyleExport();

    Ui::StyleEditor ui;
    Window *window;
    Style style;
    friend class Plugin;
};
