// Copyright (C) 2014-2018 Manuel Schneider

#include "snippeteditordialog.h"
#include<QMessageBox>

/** ***************************************************************************/
Snippets::SnippetEditorDialog::SnippetEditorDialog(QWidget *parent) : QDialog(parent) {
    ui.setupUi(this);
}


/** ***************************************************************************/
Snippets::SnippetEditorDialog::~SnippetEditorDialog() {

}


/** ***************************************************************************/
void Snippets::SnippetEditorDialog::accept()
{
    if (ui.lineEdit->text().trimmed().isEmpty())
        QMessageBox::information(this, tr("Empty title."),
                                 tr("The title field must not be empty."));
    else
        QDialog::accept();
}
