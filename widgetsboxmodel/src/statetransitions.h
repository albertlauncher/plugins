// Copyright (c) 2024-2025 Manuel Schneider

#pragma once
#include <QEvent>
#include <QSignalTransition>
#include <QState>
class QKeyEvent;


// Event transitions -------------------------------------------------------------------------------

QAbstractTransition *addTransition(QState *source, QState *target, int event_type);

QAbstractTransition *addTransition(QState *source, QState *target, int event_type, std::function<bool()> guard);

QAbstractTransition *addTransition(QState *source, QState *target, QObject *object, QEvent::Type type);

QAbstractTransition *addTransition(QState *source, QState *target, QObject *object, QEvent::Type type, std::function<bool()> guard);

QAbstractTransition *addTransition(QState *source, QState *target, QObject *object, QEvent::Type type, std::function<bool(QEvent*)> guard);

QAbstractTransition *addTransition(QState *source, QState *target, QObject *object, QEvent::Type type, std::function<bool(QKeyEvent*)> guard);

QAbstractTransition *addTransition(QState *source, QState *target, QObject *object, QEvent::Type type, int key);

QAbstractTransition *addTransition(QState *source, QState *target, QObject *object, QEvent::Type type, int key, std::function<bool()> guard);


// Signal transitions ------------------------------------------------------------------------------

template<typename T>
struct GuardedTransition : public T
{
    static_assert(std::is_base_of<QAbstractTransition, T>::value
                      && !std::is_same<QAbstractTransition, T>::value,
                  "T must be a _subclass_ of QAbstractTransition.");


    template<class... Args>
    GuardedTransition(std::function<bool()> g, Args &&...args) :
        T(std::forward<Args>(args)...),
        guard(g)
    {}

    bool eventTest(QEvent *e) override { return T::eventTest(e) && guard(); }

    const std::function<bool()> guard;

};

template<typename Func>
static QAbstractTransition *addTransition(QState *source, QState *target,
                          const typename QtPrivate::FunctionPointer<Func>::Object *sender,
                          Func signal)
{
    auto *t = new QSignalTransition(sender, signal, source);
    t->setTargetState(target);
    return t;
}

template<typename Func>
static QAbstractTransition *addTransition(QState *source, QState *target,
                          const typename QtPrivate::FunctionPointer<Func>::Object *sender,
                          Func signal,
                          std::function<bool()> guard)
{
    auto *t = new GuardedTransition<QSignalTransition>(guard, sender, signal, source);
    t->setTargetState(target);
    return t;
}
