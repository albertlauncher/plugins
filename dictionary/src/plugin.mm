// Copyright (c) 2022-2024 Manuel Schneider

#include "plugin.h"
#include <CoreServices/CoreServices.h>
#include <Foundation/Foundation.h>
#include <albert/logging.h>
#include <albert/standarditem.h>
#include <albert/util.h>
ALBERT_LOGGING_CATEGORY("dictionary")
using namespace albert;
using namespace std;

// google DCSGetActiveDictionaries
extern "C" {
  NSArray *DCSGetActiveDictionaries();
  NSArray *DCSCopyAvailableDictionaries();
  NSString *DCSDictionaryGetName(DCSDictionaryRef dictID);
  NSString *DCSDictionaryGetShortName(DCSDictionaryRef dictID);
}

QString Plugin::defaultTrigger() const { return QStringLiteral("def "); }

void Plugin::handleTriggerQuery(Query *query)
{
    CFStringRef word = (__bridge CFStringRef)query->string().toNSString();

    for (NSObject *ns_object in DCSGetActiveDictionaries())
        @autoreleasepool {
        DCSDictionaryRef dict = (__bridge DCSDictionaryRef)ns_object;

        auto long_name = QString::fromNSString(DCSDictionaryGetName(dict));

        NSString *result = (__bridge NSString *)DCSCopyTextDefinition(
            dict, word, DCSGetTermRangeInString(nil, word, 0));

        if (result) {
            auto text = QString::fromNSString(result);
            query->add(StandardItem::make(
                id(), long_name, text, icon_urls,
                {
                    {
                        "open", tr("Open in dictionary"),
                        [s = query->string()]() { openUrl("dict://" + s); }
                    }
                }
            ));
        }
    }
}
