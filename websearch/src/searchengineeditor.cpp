// Copyright (C) 2014-2018 Manuel Schneider

#include <QFileDialog>
#include <QStandardPaths>
#include "searchengineeditor.h"



/** ***************************************************************************/
Websearch::SearchEngineEditor::SearchEngineEditor(const SearchEngine &searchEngine, QWidget *parent)
    : QDialog(parent), searchEngine_(searchEngine) {

    ui.setupUi(this);
    setWindowModality(Qt::WindowModal);

    ui.lineEdit_name->setText(searchEngine.name);
    ui.lineEdit_trigger->setText(searchEngine.trigger);
    ui.lineEdit_url->setText(searchEngine.url);
    ui.toolButton_icon->setIcon(QIcon(searchEngine.iconPath));

    connect(ui.lineEdit_name, &QLineEdit::textChanged,
            [this](const QString & text){ searchEngine_.name = text; });

    connect(ui.lineEdit_trigger, &QLineEdit::textChanged,
            [this](const QString & text){ searchEngine_.trigger = text; });

    connect(ui.lineEdit_url, &QLineEdit::textChanged,
            [this](const QString & text){ searchEngine_.url = text; });

    connect(ui.toolButton_icon, &QToolButton::clicked,
            [this](){

        QString fileName =
                QFileDialog::getOpenFileName(
                    this,
                    tr("Choose icon"),
                    QStandardPaths::writableLocation(QStandardPaths::HomeLocation),
                    tr("Images (*.png *.svg)"));

        if(fileName.isEmpty())
            return;

        searchEngine_.iconPath = fileName;
        ui.toolButton_icon->setIcon(QIcon(fileName));
    });

}
