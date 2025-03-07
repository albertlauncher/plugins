// Copyright (c) 2024 Manuel Schneider

#include "statetransitions.h"


CustomEventTransition::CustomEventTransition(QEvent::Type type, QState *source) :
    QAbstractTransition(source),
    type(type)
{}

void CustomEventTransition::onTransition(QEvent *) {}

bool CustomEventTransition::eventTest(QEvent *e) { return type == e->type(); }

void addTransition(QState *source, QState *target,
                   QAbstractTransition *transition)
{
    transition->setTargetState(target);
    source->addTransition(transition);
}

void addTransition(QState *source, QState *target,
                   QEvent::Type type)
{
    auto *t = new CustomEventTransition(type, source);
    t->setTargetState(target);
}

void addTransition(QState *source, QState *target,
                   QEvent::Type type,
                   std::function<bool()> guard)
{
    auto *t = new GuardedTransition<CustomEventTransition>(guard, type, source);
    t->setTargetState(target);
}

void addTransition(QState *source, QState *target,
                   QObject *object, QEvent::Type type, int key)
{
    auto *t = new QKeyEventTransition(object, type, key, source);
    t->setTargetState(target);
}

void addTransition(QState *source, QState *target, QObject *object,
                   QEvent::Type type, int key,
                   std::function<bool()> guard)
{
    auto *t = new GuardedTransition<QKeyEventTransition>(guard, object, type, key, source);
    t->setTargetState(target);
}

void addTransition(QState *source, QState *target,
                   QObject *object, QEvent::Type type)
{
    auto *t = new QEventTransition(object, type, source);
    t->setTargetState(target);
}

void addTransition(QState *source, QState *target,
                   QObject *object, QEvent::Type type,
                   std::function<bool()> guard)
{
    auto *t = new GuardedTransition<QEventTransition>(guard, object, type, source);
    t->setTargetState(target);
}
