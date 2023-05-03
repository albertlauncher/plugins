// Copyright (c) 2022 Manuel Schneider

#include <chrono>
#include <thread>
#include "plugin.h"
#include "albert/logging.h"
using namespace std;
ALBERT_LOGGING

Plugin::Plugin() { INFO << "'Debug' created."; }

Plugin::~Plugin() { INFO << "'Debug' destroyed."; }

QString Plugin::synopsis() const { return "debug-debug-debug-debug"; }

QString Plugin::name() const { return "Debug"; }

QString Plugin::description() const { return tr("Check the API functionality."); }

bool Plugin::allowTriggerRemap() const { return false; }

void Plugin::handleTriggerQuery(TriggerQuery &query) const
{
    for(int i = 0; query.isValid() && i < 7; ++i) {
        for (int nap = 0; nap < 300; ++nap){
            if (!query.isValid())
                return;
            this_thread::sleep_for(chrono::milliseconds(10));
        }
        query.add(albert::StandardItem::make(
            QString(),
            QString("Item #%1").arg(i),
            QString("Wow, Item #%1").arg(i),
            QString("%1Item #%2").arg(query.trigger()).arg(i),
            {":debug"},
            {
                    {"Debug","Open website", [](){ albert::openWebsite(); }},
                    {"Debug","Open settings", [](){ albert::showSettings(); }}
            }
        ));
    }
}
