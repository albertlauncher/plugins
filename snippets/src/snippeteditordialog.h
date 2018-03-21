// Copyright (C) 2014-2018 Manuel Schneider

#pragma once
#include "ui_snippeteditordialog.h"

namespace Snippets {
class SnippetEditorDialog final : public QDialog
{
    Q_OBJECT

public:

    SnippetEditorDialog(QWidget *parent = nullptr);
    ~SnippetEditorDialog();
    Ui::SnippetEditorDialog ui;

};
}
