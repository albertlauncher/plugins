// Copyright (c) 2022 Manuel Schneider

#include <QKeyEvent>
#include <QMimeDatabase>
#include <QSortFilterProxyModel>
#include <QStandardItemModel>
#include <QString>
#include <QtConcurrent>
#include <QPushButton>
#include <map>
#include "mimefilterdialog.h"
using namespace std;


MimeFilterDialog::MimeFilterDialog(const QStringList &filters, QWidget *parent): QDialog(parent)
{
    ui.setupUi(this);

    // Let the listview intercept the input of the filter line edit (for navigation and activation)
    ui.lineEdit->installEventFilter(this);
    ui.listView_mimeTypes->installEventFilter(this);

    /*
     * mime list view
     */

    auto *standardItemModel = new QStandardItemModel(this);
    QList<QStandardItem*> items;
    for (const QMimeType& mimeType : QMimeDatabase().allMimeTypes()) {
        auto *item = new QStandardItem();
        if (mimeType.filterString().isEmpty())
            item->setText(mimeType.name());
        else{
            item->setText(QString("%1 - %2").arg(mimeType.name(), mimeType.filterString()));
            item->setToolTip(mimeType.filterString());
        }
        item->setData(mimeType.name(), Qt::UserRole);
        items.append(item);
    }
    standardItemModel->appendColumn(items);
    standardItemModel->sort(0);

    // Add a proxy model for mimtype filtering
    auto *proxy_model = new QSortFilterProxyModel(this);
    proxy_model->setSourceModel(standardItemModel);
    proxy_model->setFilterKeyColumn(-1); // search all cols
    ui.listView_mimeTypes->setModel(proxy_model);

    // Set the filter for the proxymodel if the users typed something
    connect(ui.lineEdit, &QLineEdit::textChanged,
            proxy_model, &QSortFilterProxyModel::setFilterFixedString);

    // On mimetype list activation add the mimetype to the filter list
    connect(ui.listView_mimeTypes, &QListView::activated, [this](const QModelIndex &index){
        ui.plainTextEdit->appendPlainText(index.data(Qt::UserRole).toString());
    });

    /*
     * mime text edit
     */

    ui.plainTextEdit->setPlainText(filters.join("\n"));

    connect(ui.plainTextEdit, &QPlainTextEdit::textChanged, this, [this](){
        auto patterns = ui.plainTextEdit->toPlainText().split("\n");
        QStringList errors;
        for (auto &pattern : patterns)
            if (auto re = QRegularExpression::fromWildcard(pattern); !re.isValid())
                errors << QString("'%1' %2").arg(pattern, re.errorString());
        ui.buttonBox->button(QDialogButtonBox::Ok)->setEnabled(errors.isEmpty());
        ui.label_error->setText(errors.join(", "));
    });
}

QStringList MimeFilterDialog::filters() const
{
    return ui.plainTextEdit->toPlainText().split("\n", Qt::SkipEmptyParts);
}

bool MimeFilterDialog::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == ui.lineEdit){
        if (event->type() == QEvent::KeyPress) {
            QKeyEvent *keyEvent = static_cast<QKeyEvent*>(event);
            switch (keyEvent->key()) {
                case Qt::Key_Up:
                case Qt::Key_Down:
                    QApplication::sendEvent(ui.listView_mimeTypes, keyEvent);
                    return true;
                case Qt::Key_Enter:
                case Qt::Key_Return:
#ifdef Q_OS_MAC
                    if (ui.listView_mimeTypes->currentIndex().isValid())
                            emit ui.listView_mimeTypes->activated(ui.listView_mimeTypes->currentIndex());
#else
                    QApplication::sendEvent(ui.listView_mimeTypes, keyEvent);
#endif
                    return true;
                default:
                    return false;
            }
        }
    }

#ifdef Q_OS_MAC
    if (watched == ui.listView_mimeTypes){
        if (event->type() == QEvent::KeyPress) {
            QKeyEvent *keyEvent = static_cast<QKeyEvent*>(event);
            switch (keyEvent->key()) {
                case Qt::Key_Enter:
                case Qt::Key_Return:
                    if (ui.listView_mimeTypes->currentIndex().isValid())
                            emit ui.listView_mimeTypes->activated(ui.listView_mimeTypes->currentIndex());
                    return true;
                default:
                    return false;
            }
        }
    }
#endif
    return false;
}

void MimeFilterDialog::keyPressEvent(QKeyEvent *evt)
{
    // Eat keys for desired behaviour
    switch ( evt->key() ) {
    case Qt::Key_Enter:
    case Qt::Key_Return:
    case Qt::Key_Escape:
        return;
    }
    QDialog::keyPressEvent(evt);
}

