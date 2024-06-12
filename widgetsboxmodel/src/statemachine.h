// Copyright (c) 2024 Manuel Schneider

#pragma once
#include <QStateMachine>
#include <QTimer>
class Window;

class StateMachine : public QStateMachine
{
public:

    enum EventType {
        QueryChanged = QEvent::User,
        QueryFinished,
        EnterActionMode,
        ExitActionMode,
        EnterFallbackMode,
        ExitFallbackMode,
    };

    class Event : public QEvent
    {
    public:
        Event(EventType eventType) : QEvent((QEvent::Type)eventType) {}
    };

    StateMachine(Window*);


    bool isActionModeActive();
    void enterActionMode();
    void exitActionMode();
    void toggleActionMode();

    bool isFallbackModeActive();
    void enterFallbackMode();
    void exitFallbackMode();
    void toggleFallbackMode();

    QState root;
    QState settings_button;
    QState settings_button_hidden;
    QState settings_button_visible;
    QState settings_button_highlight;
    QState results;
    QState results_query_unset;
    QState results_query_set;
    QState results_hidden;
    QState results_disabled;
    QState results_matches;
    QState results_fallbacks;
    QState results_match_items;
    QState results_match_actions;
    QState results_fallback_items;
    QState results_fallback_actions;

private:

    void postCustomEvent(EventType type);

    QTimer display_delay_timer;
};
