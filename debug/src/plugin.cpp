// Copyright (c) 2023 Manuel Schneider

#include "plugin.h"
#include <albert/albert.h>
#include <albert/logging.h>
#include <albert/notification.h>
#include <albert/standarditem.h>
#include <chrono>
#include <thread>
ALBERT_LOGGING_CATEGORY("debug")
using namespace albert;
using namespace std;

static auto icon = {QStringLiteral("qsp:SP_MessageBoxWarning")};

Plugin::Plugin() { DEBG << "'Debug' created."; }

Plugin::~Plugin() { DEBG << "'Debug' destroyed."; }

QString Plugin::synopsis(const QString &) const { return "debug-debug-debug-debug"; }

bool Plugin::allowTriggerRemap() const { return false; }

void Plugin::handleTriggerQuery(Query &query)
{
    if (query.string() == QStringLiteral("busy"))
    {
        for(int i = 0; query.isValid() && i < 3; ++i)
        {
            for (int nap = 0; nap < 100; ++nap){
                if (!query.isValid())
                    return;
                this_thread::sleep_for(chrono::milliseconds(10));
            }
            query.add(
                StandardItem::make(
                    {}, QString("Item #%1").arg(i), QString("Wow, Item #%1").arg(i), icon,
                    {
                        {"Debug","Open website", [](){ openWebsite(); }}
                    }
                )
            );
        }
        return;
    }

    if (QStringLiteral("notification").startsWith(query.string()))
    {
        query.add(StandardItem::make(
            {}, "Memleaking notification", "Leaks memory", icon,
            {
                {
                    "Debug", "Open website",
                    [](){
                        auto *n = new Notification;
                        n->setTitle("Memory leak");
                        n->setText("Ok.");
                        n->send();
                    }
                }
            }
        ));
    }

    if (QStringLiteral("busy").startsWith(query.string()))
    {
        query.add(StandardItem::make({}, "busy", "Test delayed queries", "busy", icon, {}));
    }
}
