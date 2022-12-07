// Copyright (c) 2022 Manuel Schneider

#include "namefilterdialog.h"
#include <QRegularExpression>
#include <QPushButton>

NameFilterDialog::NameFilterDialog(const QStringList &filters, QWidget *parent): QDialog(parent)
{
    ui.setupUi(this);
    ui.plainTextEdit->setPlainText(filters.join('\n'));
    connect(ui.plainTextEdit, &QPlainTextEdit::textChanged, this, [this](){
        auto patterns = ui.plainTextEdit->toPlainText().split("\n");
        QStringList errors;
        for (auto &pattern : patterns)
            if (QRegularExpression re(pattern); !re.isValid())
                errors << QString("'%1' %2").arg(pattern, re.errorString());

        ui.buttonBox->button(QDialogButtonBox::Ok)->setEnabled(errors.isEmpty());

        ui.label_error->setText(errors.join(", "));
    });
}

QStringList NameFilterDialog::filters() const
{
    return ui.plainTextEdit->toPlainText().split("\n", Qt::SkipEmptyParts);
}
