// Copyright (c) 2023-2024 Manuel Schneider

#pragma once
#include <QQmlEngine>
#include <QQuickWindow>
#include <albert/inputhistory.h>
class ImageProvider;
class QmlInterface;
namespace albert { class Query; }


class Window : public QQuickWindow
{
    Q_OBJECT
public:

    Window(QmlInterface &qmlif);
    ~Window();

    QString inputText() const;
    void setInputText(const QString &input);

    // Properties

    bool always_on_top;
    bool clear_on_hide;
    bool follow_mouse;
    bool hide_on_close;
    bool hide_on_focus_loss;
    bool show_centered;

    QObject *getStyleObject() const;

protected:

    bool event(QEvent *event) override;

    void loadRootComponent(const QString &path);

    bool syntheticVimNavigation(QKeyEvent*);
    bool syntheticEmacsNavigation(QKeyEvent*);
    bool sendSyntheticKey(QKeyEvent*, int);


    albert::InputHistory history_;
    ImageProvider *image_provider_; // weak ref, owned by qmlengine
    QQmlEngine engine_;
    std::unique_ptr<QQuickItem> root_object_;

    bool cm_vim = true;
    bool cm_emacs = true;


signals:

    void inputTextChanged(QString);
};







// void loadTheme();

// void setVisible(bool visible);


// bool history_search_enabled;
// bool display_scrollbar;
// uint max_results;





// Preferences

// QStringList settableThemeProperties();
// QVariant property(const QString &name) const;
// void setProperty(const QString &name, const QVariant &value);

// QFileInfoList availableThemes() const;
// void applyTheme(const QString &theme_file_path);
// void saveThemeAsFile(const QString &theme_name);

