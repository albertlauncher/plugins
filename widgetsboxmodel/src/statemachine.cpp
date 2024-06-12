// Copyright (c) 2024 Manuel Schneider

#include "statemachine.h"
#include "window.h"
#include <QEventTransition>
#include <QSignalTransition>
#include <albert/logging.h>
#include <albert/query.h>
using namespace std;


template<typename T>
class GuardedTransition : public T
{
public:
    static_assert(std::is_convertible<T*, QAbstractTransition*>::value,
                  "GuardedTransition must inherit QAbstractTransition");

    template <class... Args>
    GuardedTransition(function<bool()> g, Args&&... args)
        : T(std::forward<Args>(args)...), guard(g) {}

    bool eventTest(QEvent *e) override
    { return T::eventTest(e) && guard(); }

private:
    function<bool()> guard;
};

struct CustomEventTransition : public QAbstractTransition
{
    CustomEventTransition(StateMachine::EventType type, QState *src):
        QAbstractTransition(src), type(static_cast<QEvent::Type>(type)){}

    bool eventTest(QEvent *e) override
    { return type == e->type(); }

    void onTransition(QEvent *) override {}

    QEvent::Type type;
};

static void addCustomEventTransition(QState &src, QState &tgt,
                                     StateMachine::EventType type, function<bool()> guard = nullptr)
{
    QAbstractTransition *t;
    if (guard)
        t = new GuardedTransition<CustomEventTransition>(guard, type, &src);
    else
        t = new CustomEventTransition(type, &src);
    t->setTargetState(&tgt);
}

static void addEventTransition(QState &src, QState &tgt,
                               QObject *object, QEvent::Type type,
                               function<bool()> guard = nullptr)
{
    QAbstractTransition *t;
    if (guard)
        t = new GuardedTransition<QEventTransition>(guard, object, type, &src);
    else
        t = new QEventTransition(object, type, &src);
    t->setTargetState(&tgt);
}

template <typename Func>
static void addSignalTransition(QState &src, QState &tgt,
                                const typename QtPrivate::FunctionPointer<Func>::Object *sender,
                                Func signal,
                                function<bool()> guard = nullptr)
{
    QAbstractTransition *t;
    if (guard)
        t = new GuardedTransition<QSignalTransition>(guard, sender, signal, &src);
    else
        t = new QSignalTransition(sender, signal, &src);
    t->setTargetState(&tgt);
}


StateMachine::StateMachine(Window *window):
    root(QState::ParallelStates),
    settings_button(&root),
    settings_button_hidden(&settings_button),
    settings_button_visible(&settings_button),
    settings_button_highlight(&settings_button),
    results(&root),
    results_query_unset(&results),
    results_query_set(&results),
    results_hidden(&results_query_set),
    results_disabled(&results_query_set),
    results_matches(&results_query_set),
    results_fallbacks(&results_query_set),
    results_match_items(&results_matches),
    results_match_actions(&results_matches),
    results_fallback_items(&results_fallbacks),
    results_fallback_actions(&results_fallbacks),
    display_delay_timer(this)
{
    settings_button.setInitialState(&settings_button_hidden);
    results.setInitialState(&results_query_unset);
    results_query_set.setInitialState(&results_hidden);
    results_matches.setInitialState(&results_match_items);
    results_fallbacks.setInitialState(&results_fallback_items);
    display_delay_timer.setInterval(250);
    display_delay_timer.setSingleShot(true);
    addState(&root);
    setInitialState(&root);


    // settingsbutton hidden ->

    addEventTransition(
        settings_button_hidden, settings_button_visible,
        &window->input_frame, QEvent::Enter);

    addSignalTransition(
        settings_button_hidden, settings_button_highlight,
        window, &Window::queryChanged,
        [=] { return window->current_query != nullptr; });

    addEventTransition(
        settings_button_hidden, settings_button_highlight,
        &window->settings_button, QEvent::Enter);


    // settingsbutton visible ->

    addEventTransition(
        settings_button_visible, settings_button_hidden,
        &window->input_frame, QEvent::Leave);

    addEventTransition(
        settings_button_visible, settings_button_hidden,
        window, QEvent::WindowDeactivate);

    addSignalTransition(
        settings_button_visible, settings_button_highlight,
        window, &Window::queryChanged,
        [=] { return window->current_query != nullptr; });

    addEventTransition(
        settings_button_visible, settings_button_highlight,
        &window->settings_button, QEvent::Enter);


    // settingsbutton highlight -> visible (frame under mouse)

    addEventTransition(
        settings_button_highlight, settings_button_visible,
        &window->settings_button, QEvent::Leave,
        [=] { return window->input_frame.underMouse()
                     && (window->current_query == nullptr
                        || window->current_query->isFinished()); });

    addSignalTransition(
        settings_button_highlight, settings_button_visible,
        window, &Window::queryChanged,
        [=] { return window->input_frame.underMouse()
                     && window->current_query == nullptr
                     && !window->settings_button.underMouse(); });

    addSignalTransition(
        settings_button_highlight, settings_button_visible,
        window, &Window::queryFinished,
        [=] { return window->input_frame.underMouse()
                     && !window->settings_button.underMouse(); });


    // settingsbutton highlight -> visible (not frame under mouse)

    addEventTransition(
        settings_button_highlight, settings_button_hidden,
        &window->settings_button, QEvent::Leave,
        [=] { return !window->input_frame.underMouse()
                     && (window->current_query == nullptr
                         || window->current_query->isFinished()); });

    addSignalTransition(
        settings_button_highlight, settings_button_hidden,
        window, &Window::queryChanged,
        [=] { return !window->input_frame.underMouse()
                     && window->current_query == nullptr
                     && !window->settings_button.underMouse(); });

    addSignalTransition(
        settings_button_highlight, settings_button_hidden,
        window, &Window::queryFinished,
        [=] { return !window->input_frame.underMouse()
                     && !window->settings_button.underMouse(); });

    // underMouse does not return true on activation
    // addEventTransition(
    //     settings_button_hidden, settings_button_shown,
    //     window, QEvent::WindowActivate,
    //     [=]{ return window->input_frame.underMouse(); });


    // Query

    addSignalTransition(
        results_query_unset, results_query_set,
        window, &Window::queryChanged,
        [=]{ return window->current_query != nullptr; });

    addSignalTransition(
        results_query_set, results_query_unset,
        window, &Window::queryChanged,
        [=]{ return window->current_query == nullptr; });


    // hidden ->

    addSignalTransition(
        results_hidden, results_matches,
        window, &Window::queryMatchesAdded);

    addCustomEventTransition(
        results_hidden, results_fallbacks,
        EnterFallbackMode,
        [=]{ return window->current_query->fallbacks()->rowCount() > 0; });

    addSignalTransition(
        results_hidden, results_fallbacks,
        window, &Window::queryFinished,
        [=]{ return window->current_query->fallbacks()->rowCount() > 0
                    && !window->current_query->isTriggered(); });


    // disabled ->

    addSignalTransition(
        results_disabled, results_hidden,
        &display_delay_timer, &QTimer::timeout);

    addSignalTransition(
        results_disabled, results_matches,
        window, &Window::queryMatchesAdded);

    addSignalTransition(
        results_disabled, results_hidden,
        window, &Window::queryFinished,
        [=]{ return window->current_query->isTriggered()
                    || window->current_query->fallbacks()->rowCount() == 0; });

    addSignalTransition(
        results_disabled, results_fallbacks,
        window, &Window::queryFinished,
        [=]{ return !window->current_query->isTriggered()
                    && window->current_query->fallbacks()->rowCount() > 0; });


    // matches ->

    addSignalTransition(
        results_matches, results_disabled,
        window, &Window::queryChanged,
        [=]{ return window->current_query != nullptr; });


    addCustomEventTransition(
        results_matches, results_fallbacks,
        EnterFallbackMode,
        [=]{ return window->current_query->fallbacks()->rowCount() > 0; });

    addCustomEventTransition(results_match_items, results_match_actions, EnterActionMode);
    addCustomEventTransition(results_match_actions, results_match_items, ExitActionMode);


    // fallbacks ->

    addSignalTransition(
        results_fallbacks, results_disabled,
        window, &Window::queryChanged,
        [=]{ return window->current_query != nullptr; });

    addCustomEventTransition(
        results_fallbacks, results_hidden,
        ExitFallbackMode,
        [=]{ return window->current_query->matches()->rowCount() == 0
                        && !window->current_query->isFinished(); });

    addCustomEventTransition(
        results_fallbacks, results_matches,
        ExitFallbackMode,
        [=]{ return window->current_query->matches()->rowCount() > 0; });

    addCustomEventTransition(results_fallback_items, results_fallback_actions, EnterActionMode);
    addCustomEventTransition(results_fallback_actions, results_fallback_items, ExitActionMode);

    QObject::connect(&results_disabled, &QState::entered,
                     &display_delay_timer, static_cast<void(QTimer::*)()>(&QTimer::start));
    QObject::connect(&results_disabled, &QState::exited,
                     &display_delay_timer, &QTimer::stop);

#if 1
    QObject::connect(&settings_button_hidden,          &QState::entered, this, [](){ DEBG << "STATEMACHINE: settings_button_hidden"; });
    QObject::connect(&settings_button_visible,         &QState::entered, this, [](){ DEBG << "STATEMACHINE: settings_button_visible"; });
    QObject::connect(&settings_button_highlight,       &QState::entered, this, [](){ DEBG << "STATEMACHINE: settings_button_highlight"; });
    QObject::connect(&results_query_unset,             &QState::entered, this, [](){ DEBG << "STATEMACHINE: results_query_unset"; });
    QObject::connect(&results_query_set,               &QState::entered, this, [](){ DEBG << "STATEMACHINE: results_query_set"; });
    QObject::connect(&results_hidden,                  &QState::entered, this, [](){ DEBG << "STATEMACHINE: results_hidden"; });
    QObject::connect(&results_disabled,                &QState::entered, this, [](){ DEBG << "STATEMACHINE: results_disabled"; });
    QObject::connect(&results_match_items,             &QState::entered, this, [](){ DEBG << "STATEMACHINE: results_match_items"; });
    QObject::connect(&results_match_actions,           &QState::entered, this, [](){ DEBG << "STATEMACHINE: results_match_actions"; });
    QObject::connect(&results_fallback_items,          &QState::entered, this, [](){ DEBG << "STATEMACHINE: results_fallback_items"; });
    QObject::connect(&results_fallback_actions,        &QState::entered, this, [](){ DEBG << "STATEMACHINE: results_fallback_actions"; });
    QObject::connect(window, &Window::queryChanged, this, [=](){ DEBG << "STATEMACHINE: SIG queryChanged" << window->current_query; });
    QObject::connect(window, &Window::queryMatchesAdded, this, [](){ DEBG << "STATEMACHINE: SIG queryMatchesAdded"; });
    QObject::connect(window, &Window::queryFinished, this, [](){ DEBG << "STATEMACHINE: SIG queryFinished"; });
#endif
}

bool StateMachine::isActionModeActive()
{ return results_match_actions.active() || results_fallback_actions.active(); }

void StateMachine::enterActionMode()
{postCustomEvent(EnterActionMode); }

void StateMachine::exitActionMode()
{ postCustomEvent(ExitActionMode); }

void StateMachine::toggleActionMode()
{ isActionModeActive() ? exitActionMode() : enterActionMode(); }

bool StateMachine::isFallbackModeActive()
{ return results_fallbacks.active(); }

void StateMachine::enterFallbackMode()
{ postCustomEvent(EnterFallbackMode); }

void StateMachine::exitFallbackMode()
{ postCustomEvent(ExitFallbackMode); }

void StateMachine::toggleFallbackMode()
{ isFallbackModeActive() ? exitFallbackMode() : enterFallbackMode(); }

void StateMachine::postCustomEvent(EventType type)
{ postEvent(new Event(type)); } // takes ownership

