// Copyright (C) 2014-2018 Manuel Schneider

#include <QCheckBox>
#include <QComboBox>
#include <QDir>
#include <QFileInfo>
#include <QMessageBox>
#include <QStandardPaths>
#include <QSpinBox>
#include "configwidget.h"
#include "frontendwidget.h"
#include "ui_configwidget.h"
using namespace Core;

class WidgetBoxModel::ConfigWidget::Private {
public:
    WidgetBoxModel::Ui::ConfigWidget ui;
    FrontendWidget *frontend = nullptr;
};


/** ***********************************************************************************************/
WidgetBoxModel::ConfigWidget::ConfigWidget(FrontendWidget *frontend, QWidget *parent)
    : QWidget(parent), d(new Private) {

    d->ui.setupUi(this);

    d->frontend = frontend;

    // ALWAYS CENTER
    d->ui.checkBox_center->setChecked(d->frontend->showCentered());
    connect(d->ui.checkBox_center, &QCheckBox::toggled,
            d->frontend, &FrontendWidget::setShowCentered);

    // ALWAYS ON TOP
    d->ui.checkBox_onTop->setChecked(d->frontend->alwaysOnTop());
    connect(d->ui.checkBox_onTop, &QCheckBox::toggled,
            d->frontend, &FrontendWidget::setAlwaysOnTop);

    // HIDE ON FOCUS OUT
    d->ui.checkBox_hideOnFocusOut->setChecked(d->frontend->hideOnFocusLoss());
    connect(d->ui.checkBox_hideOnFocusOut, &QCheckBox::toggled,
            d->frontend, &FrontendWidget::setHideOnFocusLoss);

    // HIDE ON CLOSE
    d->ui.checkBox_hideOnClose->setChecked(d->frontend->hideOnClose());
    connect(d->ui.checkBox_hideOnClose, &QCheckBox::toggled,
            d->frontend, &FrontendWidget::setHideOnClose);

    // CLEAR ON HIDE
    d->ui.checkBox_clearOnHide->setChecked(d->frontend->clearOnHide());
    connect(d->ui.checkBox_clearOnHide, &QCheckBox::toggled,
            d->frontend, &FrontendWidget::setClearOnHide);

    // MAX RESULTS
    d->ui.spinBox_results->setValue(d->frontend->maxResults());
    connect(d->ui.spinBox_results, static_cast<void(QSpinBox::*)(int)>(&QSpinBox::valueChanged),
            d->frontend, &FrontendWidget::setMaxResults);

    // DISPLAY SCROLLBAR
    d->ui.checkBox_scrollbar->setChecked(d->frontend->displayScrollbar());
    connect(d->ui.checkBox_scrollbar, &QCheckBox::toggled,
            d->frontend, &FrontendWidget::setDisplayScrollbar);

    // DISPLAY ICONS
    d->ui.checkBox_icons->setChecked(d->frontend->displayIcons());
    connect(d->ui.checkBox_icons, &QCheckBox::toggled,
            d->frontend, &FrontendWidget::setDisplayIcons);

    // DISPLAY SHADOW
    d->ui.checkBox_shadow->setChecked(d->frontend->displayShadow());
    connect(d->ui.checkBox_shadow, &QCheckBox::toggled,
            d->frontend, &FrontendWidget::setDisplayShadow);

    // THEMES
    QStringList pluginDataPaths = QStandardPaths::locateAll(QStandardPaths::AppDataLocation,
                                                            "org.albert.frontend.widgetboxmodel",
                                                            QStandardPaths::LocateDirectory);

    QFileInfoList themes;
    for (const QString &pluginDataPath : pluginDataPaths)
        themes << QDir(QString("%1/themes").arg(pluginDataPath))
                  .entryInfoList(QStringList("*.qss"), QDir::Files | QDir::NoSymLinks);

    for (const QFileInfo &fi : themes) {
        d->ui.comboBox_themes->addItem(fi.baseName(), fi.canonicalFilePath());
        if ( fi.baseName() == d->frontend->theme())
            d->ui.comboBox_themes->setCurrentIndex(d->ui.comboBox_themes->count()-1);
    }

    connect(d->ui.comboBox_themes, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
            [this](int i){
        // Apply and save the theme
        QString currentTheme = d->frontend->theme();
        if (!d->frontend->setTheme(d->ui.comboBox_themes->itemText(i))) {
            QMessageBox(QMessageBox::Critical, "Error",
                        "Could not apply theme.",
                        QMessageBox::NoButton,
                        this).exec();
            if (!d->frontend->setTheme(currentTheme)) {
               qFatal("Rolling back theme failed.");
            }
        }
    });
}


/** ***********************************************************************************************/
WidgetBoxModel::ConfigWidget::~ConfigWidget() {
    // Needed since default dtor of unique ptr in the header has to know the type
}
