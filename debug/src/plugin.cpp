// Copyright (c) 2023 Manuel Schneider

#include "albert/albert.h"
#include "albert/extension/queryhandler/standarditem.h"
#include "albert/logging.h"
#include "plugin.h"
#include <chrono>
#include <thread>
ALBERT_LOGGING_CATEGORY("debug")
using namespace std;

Plugin::Plugin() { DEBG << "'Debug' created."; }

Plugin::~Plugin() { DEBG << "'Debug' destroyed."; }

QString Plugin::synopsis() const { return "debug-debug-debug-debug"; }

bool Plugin::allowTriggerRemap() const { return false; }

void Plugin::handleTriggerQuery(TriggerQuery *query) const
{
    for(int i = 0; query->isValid() && i < 7; ++i) {
        for (int nap = 0; nap < 300; ++nap){
            if (!query->isValid())
                return;
            this_thread::sleep_for(chrono::milliseconds(10));
        }
        query->add(albert::StandardItem::make(
            QString(),
            QString("Item #%1").arg(i),
            QString("Wow, Item #%1").arg(i),
            QString("%1Item #%2").arg(query->trigger()).arg(i),
            {"qsp:SP_MessageBoxWarning"},
            {
                    {"Debug","Open website", [](){ albert::openWebsite(); }},
                    {"Debug","Open settings", [](){ albert::showSettings(); }}
            }
        ));
    }
}
