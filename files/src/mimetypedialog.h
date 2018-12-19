// Copyright (C) 2014-2018 Manuel Schneider

#pragma once
#include <QDialog>
#include <QFuture>
#include <QStringListModel>

namespace Files{
namespace Ui {
class MimeTypeDialog;
}

class MimeTypeDialog : public QDialog
{
    Q_OBJECT

public:

    explicit MimeTypeDialog(const QStringList &filters, QWidget *parent = nullptr);
    ~MimeTypeDialog() override;

    QStringList filters() const;

protected:

    bool eventFilter(QObject *watched, QEvent *event) override;
    void keyPressEvent(QKeyEvent *evt) override;

    QStringListModel *filtersModel;
    Ui::MimeTypeDialog *ui;
    bool exit_thread;
    QFuture<void> future;

};
}
