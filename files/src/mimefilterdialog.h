// Copyright (c) 2022 Manuel Schneider

#pragma once
#include <QDialog>
#include <QStringListModel>
#include "ui_mimefilterdialog.h"

class MimeFilterDialog : public QDialog
{
    Q_OBJECT
public:
    explicit MimeFilterDialog(const QStringList &filters, QWidget *parent = nullptr);
    QStringList filters() const;
protected:
    bool eventFilter(QObject *watched, QEvent *event) override;
    void keyPressEvent(QKeyEvent *evt) override;
    Ui::MimeFilterDialog ui;
};
