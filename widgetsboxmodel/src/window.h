// Copyright (c) 2022-2024 Manuel Schneider

#pragma once
#include "actionslist.h"
#include "frame.h"
#include "inputframe.h"
#include "inputline.h"
#include "resizinglist.h"
#include "resultslist.h"
#include "roundedrect.h"
#include "settingsbutton.h"
#include "statemachine.h"
#include "style.h"
#include <QFileInfo>
#include <QLabel>
#include <QObject>
#include <QPoint>
#include <QWidget>
#include <albert/property.h>
class Plugin;
namespace albert { class Query; }

class Window : public QWidget
{
    Q_OBJECT
    friend class StateMachine;

public:

    Window(Plugin *plugin);
    ~Window();

    QString input() const;
    void setInput(const QString&);
    void setQuery(albert::Query *query);


    // const QString &lightTheme() const;
    // void setLightTheme(const QString& theme);

    // const QString &darkTheme() const;
    // void setDarkTheme(const QString& theme);

    void applyThemeFile(const QString& path);



    const Style &style();
    void setStyle(const Style &style);

    QFileInfoList findStyles() const;


private:

    std::unique_ptr<QSettings> settings() const;
    bool event(QEvent *event) override;
    bool eventFilter(QObject *watched, QEvent *event) override;
    void paintEvent(QPaintEvent *event) override;
    void setStyle(const Style *s);

    Plugin * const plugin;

    Frame window_frame;
    RoundedRect input_frame;
    InputLine input_line;
    QLabel synopsis;
    SettingsButton settings_button;
    ResultsList results_list;
    ActionsList actions_list;

    Style style_;
    QString light_style_file_;
    QString dark_style_file_;

    StateMachine state_machine;

    albert::Query *current_query;

    Qt::KeyboardModifier command_mod = Qt::ControlModifier;
    Qt::KeyboardModifier actions_mod = Qt::AltModifier;
    Qt::KeyboardModifier fallback_mod = Qt::MetaModifier;
    Qt::Key command_key= Qt::Key_Control;
    Qt::Key actions_key= Qt::Key_Alt;
    Qt::Key fallback_key= Qt::Key_Meta;

signals:

    void inputChanged(QString);
    void visibleChanged(bool);
    void queryChanged();
    void queryMatchesAdded();
    void queryFinished();

public:

    // Properties
    ALBERT_PROPERTY_GETSET(QString, light_style_file, {}, settings)
    ALBERT_PROPERTY_GETSET(QString, dark_style_file, {}, settings)
    ALBERT_PROPERTY_GETSET(bool, always_on_top, true, settings)
    ALBERT_PROPERTY_MEMBER(bool, clear_on_hide, input_line.clear_on_hide, true, settings)
    ALBERT_PROPERTY_GETSET(bool, display_client_shadow, true, settings)
    ALBERT_PROPERTY_GETSET(bool, display_system_shadow, false, settings)
    ALBERT_PROPERTY(bool, follow_cursor, true, settings)
    ALBERT_PROPERTY(bool, hide_on_focus_loss, true, settings)
    ALBERT_PROPERTY_MEMBER(bool, history_search, input_line.history_search, true, settings)
    ALBERT_PROPERTY_GETSET(uint, max_results, 5, settings)
    ALBERT_PROPERTY(bool, quit_on_close, false, settings)
    ALBERT_PROPERTY(bool, show_centered, true, settings)
};
