// Copyright (c) 2022-2025 Manuel Schneider

#pragma once
#include <QWidget>
class Plugin;
class Window;

class ConfigWidget : public QWidget
{
public:

    ConfigWidget(Plugin&, Window &);

private:

    Plugin &plugin;
    Window &window;

};
