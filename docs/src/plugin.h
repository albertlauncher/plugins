// Copyright (c) 2022-2024 Manuel Schneider

#pragma once
#include <albert/indexqueryhandler.h>
#include <albert/extensionplugin.h>
#include "ui_configwidget.h"
class QNetworkReply;

struct Docset
{
    Docset(QString source_id, QString identifier, QString title, QString icon_path);

    std::vector<albert::IndexItem> createIndexItems() const;

    const QString source_id;
    const QString identifier;
    const QString title;
    const QString icon_path;
    QString path;  // not downloaded yet if null
};


class Plugin : public albert::ExtensionPlugin,
               public albert::IndexQueryHandler
{
    ALBERT_PLUGIN

public:

    Plugin();
    ~Plugin();

    void updateIndexItems() override;
    QWidget* buildConfigWidget() override;

    void updateDocsetList();
    const std::map<QString, Docset> &docsets() const;

    void downloadDocset(const QString &name);
    void cancelDownload();
    bool isDownloading() const;
    void removeDocset(const QString &name);

private:

    void debug(const QString &);
    void error(const QString &, QWidget *modal_parent = nullptr);

    std::map<QString, Docset> docsets_;
    QNetworkReply *download_ = nullptr;

signals:

    void docsetsChanged();
    void downloadStateChanged();
    void statusInfo(const QString&);

};

class ConfigWidget final : public QWidget
{
    Q_OBJECT
public:
    ConfigWidget(Plugin *);
private:
    void updateDocsets();
    Plugin * const plugin;
    Ui::ConfigWidget ui;
};
