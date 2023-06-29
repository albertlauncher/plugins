// Copyright (c) 2023 Manuel Schneider

#pragma once
#include "albert/extension/frontend/frontend.h"
#include "albert/extension/frontend/inputhistory.h"
#include "albert/plugin.h"
#include "imageprovider.h"
#include <QFileInfo>
#include <QPoint>
#include <QQuickWindow>
#include <QSettings>
#include <memory>
#include <vector>
class QAbstractItemModel;
class QQmlEngine;
class QQuickWindow;
class QueryExecution;
class Plugin;


class QmlInterface : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QObject* currentQuery_ READ currentQuery  NOTIFY currentQueryChanged)
public:
    QmlInterface(Plugin *plugin);

    Q_INVOKABLE void showSettings();
    Q_INVOKABLE QObject *query(const QString &query);
    Q_INVOKABLE QObject *currentQuery();
    Q_INVOKABLE void clearQueries();


    Q_INVOKABLE QString kcString(int kc);

private:
    void onQueryReady();
    void onQueryFinished();

    Plugin *plugin_;
    std::vector<std::shared_ptr<albert::Query>> queries_;
    QObject *currentQuery_;

signals:
    void currentQueryChanged();
    void currentQueryReady(); // convenience signal to avoid the boilerplate in qml
    void currentQueryFinished(); // convenience signal to avoid the boilerplate in qml

};



class Plugin : public albert::plugin::ExtensionPlugin<albert::Frontend, QQuickWindow>
{
    Q_OBJECT
    ALBERT_PLUGIN
    ALBERT_PLUGIN_PROPERTY(bool, showCentered, true)
    ALBERT_PLUGIN_PROPERTY(bool, hideOnFocusLoss, true)
    ALBERT_PLUGIN_PROPERTY(bool, hideOnClose, true)
    ALBERT_PLUGIN_PROPERTY(bool, clearOnHide, true)
    ALBERT_PLUGIN_PROPERTY(bool, followMouse, true)
    ALBERT_PLUGIN_PROPERTY_NONTRIVIAL(bool, alwaysOnTop, true)
    ALBERT_PLUGIN_PROPERTY_NONTRIVIAL(bool, displaySystemShadow, false)

public:
    Plugin();
    ~Plugin();

    void initialize(albert::ExtensionRegistry*) override;

    void loadRootComponent(const QString &path);


    // Preferences

    QStringList settableThemeProperties();
    QVariant property(const QString &name) const;
    void setProperty(const QString &name, const QVariant &value);

    QFileInfoList availableThemes() const;
    void applyTheme(const QString &theme_file_path);
    void saveThemeAsFile(const QString &theme_name);

    // albert::Frontend

    bool isVisible() const override;
    void setVisible(bool visible) override;
    QString input() const override;
    void setInput(const QString&) override;
    unsigned long long winId() const override;
    QWidget* createFrontendConfigWidget() override;

protected:
    bool event(QEvent *event) override;

    bool syntheticVimNavigation(QKeyEvent*);
    bool syntheticEmacsNavigation(QKeyEvent*);
    bool sendSyntheticKey(QKeyEvent*, int);

    QObject *getThemePropertiesObject() const;
    void loadTheme();

    QPoint clickOffset_;  // The offset from cursor to topleft. Used when the window is dragged
    albert::InputHistory history_;
    ImageProvider *image_provider_; // weak ref, owned by qmlengine
    QmlInterface qml_interface_;
    QQmlEngine engine_;
    std::unique_ptr<QQuickItem> root_object_;
    QObject *input_;

    bool cm_vim = true;
    bool cm_emacs = true;
};


