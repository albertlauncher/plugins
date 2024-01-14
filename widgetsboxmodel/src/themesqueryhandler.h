// Copyright (c) 2022-2024 Manuel Schneider

#pragma once
#include "albert/extension/queryhandler/triggerqueryhandler.h"
class Window;

class ThemesQueryHandler : public albert::TriggerQueryHandler
{
public:

    ThemesQueryHandler(Window *w);

    QString id() const override;
    QString name() const override;
    QString description() const override;
    QString synopsis() const override;
    QString defaultTrigger() const override;
    void handleTriggerQuery(TriggerQuery*) const override;

private:

    Window *window;
};

