// Copyright (C) 2014-2018 Manuel Schneider

#include <QDir>
#include <QDirIterator>
#include <QDebug>
#include <QStandardPaths>
#include <QMessageBox>
#include <QShortcut>
#include <QStandardItemModel>
#include <QDesktopWidget>
#include <QFocusEvent>
#include "configwidget.h"
#include "mainwindow.h"
#include "propertyeditor.h"

/** ***************************************************************************/
QmlBoxModel::ConfigWidget::ConfigWidget(MainWindow *mainWindow, QWidget *parent, Qt::WindowFlags f)
    : QWidget(parent, f), mainWindow_(mainWindow) {

    ui.setupUi(this);

    // ALWAYS ON TOP
    ui.checkBox_onTop->setChecked(mainWindow_->alwaysOnTop());
    connect(ui.checkBox_onTop, &QCheckBox::toggled, mainWindow_,
            &MainWindow::setAlwaysOnTop);

    // HIDE ON FOCUS OUT
    ui.checkBox_hideOnFocusOut->setChecked(mainWindow_->hideOnFocusLoss());
    connect(ui.checkBox_hideOnFocusOut, &QCheckBox::toggled,
            mainWindow_, &MainWindow::setHideOnFocusLoss);

    // ALWAYS CENTER
    ui.checkBox_center->setChecked(mainWindow_->showCentered());
    connect(ui.checkBox_center, &QCheckBox::toggled,
            mainWindow_, &MainWindow::setShowCentered);

    // HIDE ON CLOSE
    ui.checkBox_hideOnClose->setChecked(mainWindow_->hideOnClose());
    connect(ui.checkBox_hideOnClose, &QCheckBox::toggled,
            mainWindow_, &MainWindow::setHideOnClose);

    // CLEAR ON HIDE
    ui.checkBox_clearOnHide->setChecked(mainWindow_->clearOnHide());
    connect(ui.checkBox_clearOnHide, &QCheckBox::toggled,
            mainWindow_, &MainWindow::setClearOnHide);


    /*
     *  STYLES
     */

    // Fill the combobox
    for ( const QmlStyleSpec &style : mainWindow_->availableStyles() ) {
        ui.comboBox_style->addItem(style.name, style.mainComponent);

        // Add tooltip
        ui.comboBox_style->setItemData(ui.comboBox_style->count()-1,
                                       QString("%1\nVersion: %2\nAuthor: %3")
                                       .arg(style.name, style.version, style.author),
                                       Qt::ToolTipRole);

        if ( style.mainComponent == mainWindow_->source().toString() )
            ui.comboBox_style->setCurrentIndex(ui.comboBox_style->count()-1);
    }
    updateThemes();

    connect(ui.comboBox_style, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
            this, &ConfigWidget::onStyleChanged);

    connect(ui.comboBox_themes, static_cast<void (QComboBox::*)(const QString &)>(&QComboBox::currentIndexChanged),
            this, &ConfigWidget::onThemeChanged);

    // PROPERTY EDITOR
    connect(ui.toolButton_propertyEditor, &QToolButton::clicked, mainWindow_, [this](){
        PropertyEditor *pe = new PropertyEditor(mainWindow_, this);
        pe->setWindowModality(Qt::WindowModality::WindowModal);
        pe->show();
    });
}


/** ***************************************************************************/
void QmlBoxModel::ConfigWidget::onStyleChanged(int i) {
    // Apply the style
    mainWindow_->setSource(mainWindow_->availableStyles()[static_cast<size_t>(i)].mainComponent);
    updateThemes();
}


/** ***************************************************************************/
void QmlBoxModel::ConfigWidget::onThemeChanged(const QString &text) {
    mainWindow_->setTheme(text);
}


/** ***************************************************************************/
void QmlBoxModel::ConfigWidget::updateThemes() {

    ui.comboBox_themes->blockSignals(true);

    ui.comboBox_themes->clear();

    // Add disabled placeholder item
    ui.comboBox_themes->addItem("Choose theme...");
    const QStandardItemModel* model = qobject_cast<const QStandardItemModel*>(ui.comboBox_themes->model());
    QStandardItem* item = model->item(0);
    item->setEnabled(false);
    ui.comboBox_themes->insertSeparator(1);

    // Add themes
    ui.comboBox_themes->addItems(mainWindow_->availableThemes());

    ui.comboBox_themes->blockSignals(false);

}
