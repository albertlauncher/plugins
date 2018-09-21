// Copyright (C) 2014-2018 Manuel Schneider

#include <QStandardPaths>
#include "configwidget.h"

/** ***************************************************************************/
Applications::ConfigWidget::ConfigWidget(QWidget *parent) : QWidget(parent) {
    ui.setupUi(this);

    // Show the app dirs in the label
    QStringList standardPaths = QStandardPaths::standardLocations(QStandardPaths::ApplicationsLocation);
    ui.label->setText(ui.label->text().replace("__XDG_DATA_DIRS__", standardPaths.join(", ")));
}
