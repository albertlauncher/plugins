// Copyright (C) 2014-2018 Manuel Schneider

#pragma once
#include <QQuickView>
#include <QSettings>
#include <QPoint>
#include <QFileSystemWatcher>
#include <QIdentityProxyModel>
#include "albert/util/history.h"

namespace QmlBoxModel {

struct QmlStyleSpec {
    QString name;
    QString version;
    QString author;
    QString mainComponent;
};

class FrontendPlugin;

class MainWindow final : public QQuickView
{
    Q_OBJECT

public:

    MainWindow(FrontendPlugin *plugin, QWindow *parent = 0);
    ~MainWindow();

    QString input();
    void setInput(const QString&);
    void setModel(QAbstractItemModel* model);

    const std::vector<QmlStyleSpec> &availableStyles() const;
    QStringList settableProperties();

    QVariant property(const char *name) const;
    void setProperty(const char *attribute, const QVariant &value);

    QStringList availableThemes();
    void setTheme(const QString& name);

    void setSource(const QUrl & url);

    bool showCentered() const;
    void setShowCentered(bool showCentered);

    bool hideOnFocusLoss() const;
    void setHideOnFocusLoss(bool hideOnFocusLoss);

    bool hideOnClose() const;
    void setHideOnClose(bool b = true);

    bool clearOnHide() const;
    void setClearOnHide(bool b = true);

    bool alwaysOnTop() const;
    void setAlwaysOnTop(bool alwaysOnTop);

protected:

    bool event(QEvent *event) override;
    void focusOutEvent(QFocusEvent *event) override;

    bool showCentered_;
    bool hideOnFocusLoss_;
    bool hideOnClose_;
    bool clearOnHide_;
    Core::History history_;
    QIdentityProxyModel model_;
    std::vector<QmlStyleSpec> styles_;
    QFileSystemWatcher watcher_;
    FrontendPlugin *plugin_;

signals:

    void inputChanged();
    void settingsWidgetRequested();

};

}
