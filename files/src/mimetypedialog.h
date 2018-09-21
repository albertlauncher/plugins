// Copyright (C) 2014-2018 Manuel Schneider

#pragma once
#include <QDialog>
#include <QStringListModel>

namespace Files{
namespace Ui {
class MimeTypeDialog;
}

class MimeTypeDialog : public QDialog
{
    Q_OBJECT

public:

    explicit MimeTypeDialog(const QStringList &filters, QWidget *parent = 0);
    ~MimeTypeDialog();

    QStringList filters() const;

protected:

    bool eventFilter(QObject *watched, QEvent *event) override;
    void keyPressEvent(QKeyEvent *evt) override;

    QStringListModel *filtersModel;
    Ui::MimeTypeDialog *ui;

};
}
