// Copyright (c) 2022-2025 Manuel Schneider

#include "configwidget.h"
#include "plugin.h"
#include <albert/logging.h>
ALBERT_LOGGING_CATEGORY("wbm")
using namespace albert;
using namespace std;

Plugin::Plugin() :
    window(this),
    themes_query_handler(&window)
{
    connect(&window, &Window::inputChanged, this, &Plugin::inputChanged);
    connect(&window, &Window::visibleChanged, this, &Plugin::visibleChanged);
}

vector<Extension *> Plugin::extensions() { return {&themes_query_handler}; }

QString Plugin::input() const { return window.input(); }

void Plugin::setInput(const QString &input) { window.setInput(input); }

bool Plugin::isVisible() const { return window.isVisible(); }

void Plugin::setVisible(bool visible) { window.setVisible(visible); }

QWidget *Plugin::createFrontendConfigWidget() { return new ConfigWidget(*this, window); }

unsigned long long Plugin::winId() const { return window.winId(); }

void Plugin::setQuery(Query *q) { window.setQuery(q); }
