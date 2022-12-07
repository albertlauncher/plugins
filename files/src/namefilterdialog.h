// Copyright (c) 2022 Manuel Schneider

#pragma once
#include <QDialog>
#include "ui_namefilterdialog.h"

class NameFilterDialog : public QDialog
{
    Q_OBJECT
public:
    explicit NameFilterDialog(const QStringList &filters, QWidget *parent);
    QStringList filters() const;
protected:
    Ui::NameFilterDialog ui;
};
