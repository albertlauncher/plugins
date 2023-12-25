// Copyright (c) 2023 Manuel Schneider

#include "searchengineeditor.h"
#include <QApplication>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QFileDialog>
#include <QMessageBox>
#include <QMimeData>
#include <QMimeDatabase>
#include <QStandardPaths>
#include <QToolButton>
#include <QUrl>


SearchEngineEditor::SearchEngineEditor(const QString &icon_url,
                                       const QString &name,
                                       const QString &trigger,
                                       const QString &url,
                                       QWidget *parent) : QDialog(parent)
{
    ui.setupUi(this);
    setWindowModality(Qt::WindowModal);

    ui.label_iconhint->setForegroundRole(QPalette::PlaceholderText);

    if (QUrl qurl(icon_url); qurl.isLocalFile())
        ui.toolButton_icon->setIcon(QIcon(qurl.toLocalFile()));
    else
        ui.toolButton_icon->setIcon(QIcon(icon_url));
    ui.toolButton_icon->setAcceptDrops(true);
    ui.lineEdit_name->setText(name);
    ui.lineEdit_trigger->setText(trigger);
    ui.lineEdit_url->setText(url);

    connect(ui.toolButton_icon, &QToolButton::clicked, this, [this](){

        QString fileName =
            QFileDialog::getOpenFileName(
                this,
                tr("Choose icon"),
                QStandardPaths::writableLocation(QStandardPaths::HomeLocation),
                tr("Images (*.png *.svg)"));

        if (fileName.isEmpty())
            return;

        icon_image = std::make_unique<QImage>(fileName);
        ui.toolButton_icon->setIcon(QIcon(fileName));
    });

    connect(ui.lineEdit_name, &QLineEdit::editingFinished, this,
            [&]() { ui.lineEdit_name->setText(ui.lineEdit_name->text().trimmed()); });

    connect(ui.lineEdit_trigger, &QLineEdit::editingFinished, this,
            [&]() { ui.lineEdit_trigger->setText(ui.lineEdit_trigger->text().trimmed()); });

    connect(ui.lineEdit_url, &QLineEdit::editingFinished, this,
            [&]() { ui.lineEdit_url->setText(ui.lineEdit_url->text().trimmed()); });

    disconnect(ui.buttonBox, &QDialogButtonBox::accepted,
               this, &QDialog::accept);

    connect(ui.buttonBox, &QDialogButtonBox::accepted, this, [this]() {
        if (ui.lineEdit_name->text().isEmpty()
            || ui.lineEdit_trigger->text().isEmpty()
            || ui.lineEdit_url->text().isEmpty())
            QMessageBox::warning(this, qApp->applicationDisplayName(),
                                 "None of the fields must be empty.");
        else
            accept();
    });

    ui.toolButton_icon->installEventFilter(this);
}

QString SearchEngineEditor::name() const { return ui.lineEdit_name->text(); }

QString SearchEngineEditor::trigger() const { return ui.lineEdit_trigger->text(); }

QString SearchEngineEditor::url() const { return ui.lineEdit_url->text(); }

bool SearchEngineEditor::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == ui.toolButton_icon){

        if (event->type() == QEvent::DragEnter){
            auto *e = static_cast<QDragEnterEvent*>(event);
            if (e->proposedAction() == Qt::CopyAction){
                if (e->mimeData()->hasImage()){
                    e->acceptProposedAction();
                    return true;
                } else if (e->mimeData()->hasUrls()) {
                    QMimeDatabase db;
                    for (const QUrl &url : e->mimeData()->urls()) {
                        if (url.isLocalFile() && db.mimeTypeForUrl(url).name().startsWith("image/")){
                            e->acceptProposedAction();
                            return true;
                        }
                    }
                }
            }
        }

        if (event->type() == QEvent::Drop){
            auto *e = static_cast<QDropEvent*>(event);
            if (e->proposedAction() == Qt::CopyAction){
                if (e->mimeData()->hasImage()){
                    icon_image = std::make_unique<QImage>(qvariant_cast<QImage>(e->mimeData()->imageData()));
                    ui.toolButton_icon->setIcon(QIcon(QPixmap::fromImage(*icon_image)));
                    e->acceptProposedAction();
                    return true;
                } else if (e->mimeData()->hasUrls()) {
                    QMimeDatabase db;
                    for (const QUrl &url : e->mimeData()->urls()) {
                        if (url.isLocalFile() && db.mimeTypeForUrl(url).name().startsWith("image/")){
                            icon_image = std::make_unique<QImage>(url.toLocalFile());
                            ui.toolButton_icon->setIcon(QIcon(QPixmap::fromImage(*icon_image)));
                            e->acceptProposedAction();
                            return true;
                        }
                    }
                }
            }
        }
    }
    return false;
}
