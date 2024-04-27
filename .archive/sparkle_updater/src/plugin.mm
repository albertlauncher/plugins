// Copyright (c) 2023 Manuel Schneider

#include "plugin.h"
#include "ui_configwidget.h"
#include <QDateTime>
#include <QWidget>
#include <Sparkle.h>
using namespace albert;
using namespace std;


@interface AppUpdaterDelegate : NSObject <SPUUpdaterDelegate>
@end

@implementation AppUpdaterDelegate
- (nullable NSString *)feedURLStringForUpdater:(nonnull SPUUpdater *)updater
{ return [[NSString alloc]initWithString:@"https://albertlauncher.github.io/appcast.xml"]; }
@end


class Plugin::Private
{
public:
    AppUpdaterDelegate *updaterDelegate;
    SPUStandardUpdaterController *updaterController;
};


Plugin::Plugin() : d(new Private)
{
    d->updaterDelegate = [[AppUpdaterDelegate alloc] init];
    d->updaterController = [[SPUStandardUpdaterController alloc]
        initWithStartingUpdater:YES
                updaterDelegate:d->updaterDelegate
             userDriverDelegate:nil
    ];
}

Plugin::~Plugin()
{
    [d->updaterController release];
    [d->updaterDelegate release];
}

QWidget *Plugin::buildConfigWidget()
{
    auto *widget = new QWidget;
    Ui::ConfigWidget ui;
    ui.setupUi(widget);

    ui.label_lastUpdateCheck->setText(QDateTime::fromNSDate(d->updaterController.updater.lastUpdateCheckDate).toString());
    ui.checkBox_autoUpdate->setChecked(d->updaterController.updater.automaticallyChecksForUpdates);
    ui.checkBox_autoDownload->setChecked(d->updaterController.updater.automaticallyDownloadsUpdates);

    connect(ui.checkBox_autoUpdate, &QCheckBox::clicked,
            this, [this](bool value){ d->updaterController.updater.automaticallyChecksForUpdates = value; });

    connect(ui.checkBox_autoDownload, &QCheckBox::clicked,
            this, [this](bool value){ d->updaterController.updater.automaticallyDownloadsUpdates = value; });

    connect(ui.pushButton_checkUpdate, &QPushButton::clicked, this,
            [this](){ [d->updaterController checkForUpdates:nil]; });

    return widget;
}
