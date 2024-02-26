// Copyright (c) 2023-2024 Manuel Schneider

#pragma once
#include "albert/extension/frontend/inputhistory.h"
#include <QQmlEngine>
#include <QQuickWindow>
class ImageProvider;
class QmlInterface;


class Window : public QQuickWindow
{
    Q_OBJECT
public:

    Window(QmlInterface &qmlif);
    ~Window();

    QString input() const;
    void setInput(const QString &input);

    // void setVisible(bool visible);

    // Preferences

    // QStringList settableThemeProperties();
    // QVariant property(const QString &name) const;
    // void setProperty(const QString &name, const QVariant &value);

    // QFileInfoList availableThemes() const;
    // void applyTheme(const QString &theme_file_path);
    // void saveThemeAsFile(const QString &theme_name);

    // Properties

    bool always_on_top;
    bool clear_on_hide;
    bool follow_mouse;
    bool hide_on_close;
    bool hide_on_focus_loss;
    bool show_centered;
    // bool history_search_enabled;
    // bool display_scrollbar;
    // uint max_results;

protected:

    bool event(QEvent *event) override;

    void loadRootComponent(const QString &path);

    bool syntheticVimNavigation(QKeyEvent*);
    bool syntheticEmacsNavigation(QKeyEvent*);
    bool sendSyntheticKey(QKeyEvent*, int);

    QObject *getThemePropertiesObject() const;
    void loadTheme();

    QPoint clickOffset_;  // The offset from cursor to topleft. Used when the window is dragged
    albert::InputHistory history_;
    ImageProvider *image_provider_; // weak ref, owned by qmlengine
    QQmlEngine engine_;
    std::unique_ptr<QQuickItem> root_object_;
    QObject *input_;

    bool cm_vim = true;
    bool cm_emacs = true;

signals:

    void inputChanged();
};


