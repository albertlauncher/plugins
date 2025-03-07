// Copyright (c) 2024 Manuel Schneider

#pragma once
#include <QAbstractTransition>
#include <QEvent>
#include <QEventTransition>
#include <QKeyEventTransition>
#include <QSignalTransition>
#include <QState>

template<typename T>
struct GuardedTransition : public T
{
    static_assert(std::is_convertible<T *, QAbstractTransition *>::value,
                  "GuardedTransition must inherit QAbstractTransition");

    template<class... Args>
    GuardedTransition(std::function<bool()> g, Args &&...args) :
        T(std::forward<Args>(args)...),
        guard(g)
    {}

    bool eventTest(QEvent *e) override { return T::eventTest(e) && guard(); }

    const std::function<bool()> guard;

};


void addTransition(QState *source, QState *target,
                   QAbstractTransition *transition);;


// (Guarded) Custom event transitions --------------------------------------------------------------

struct CustomEventTransition : public QAbstractTransition
{
    CustomEventTransition(QEvent::Type type, QState *source);
    void onTransition(QEvent *) override;
    bool eventTest(QEvent *e) override;
    const QEvent::Type type;
};

void addTransition(QState *source, QState *target,
                   QEvent::Type type);

void addTransition(QState *source, QState *target,
                   QEvent::Type type, std::function<bool()> guard);


// (Guarded) Key event transitions -----------------------------------------------------------------

void addTransition(QState *source, QState *target,
                   QObject *object, QEvent::Type type, int key);

void addTransition(QState *source, QState *target,
                   QObject *object, QEvent::Type type, int key,
                   std::function<bool()> guard);

// (Guarded) Event transitions ---------------------------------------------------------------------

void addTransition(QState *source, QState *target,
                   QObject *object, QEvent::Type type);

void addTransition(QState *source, QState *target,
                   QObject *object, QEvent::Type type,
                   std::function<bool()> guard);

// (Guarded) Signal transitions --------------------------------------------------------------------

template<typename Func>
static void addTransition(QState *source, QState *target,
                          const typename QtPrivate::FunctionPointer<Func>::Object *sender,
                          Func signal)
{
    auto *t = new QSignalTransition(sender, signal, source);
    t->setTargetState(target);
}

template<typename Func>
static void addTransition(QState *source, QState *target,
                          const typename QtPrivate::FunctionPointer<Func>::Object *sender,
                          Func signal,
                          std::function<bool()> guard)
{
    auto *t = new GuardedTransition<QSignalTransition>(guard, sender, signal, source);
    t->setTargetState(target);
}













































