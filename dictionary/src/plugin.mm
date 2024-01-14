// Copyright (c) 2022-2024 Manuel Schneider

#include "albert/albert.h"
#include "albert/extension/queryhandler/standarditem.h"
#include "albert/logging.h"
#include "plugin.h"
#include <Cocoa/Cocoa.h>
#include <CoreServices/CoreServices.h>
ALBERT_LOGGING_CATEGORY("dictionary")
using namespace std;
using namespace albert;

// google DCSGetActiveDictionaries
extern "C" {
  NSArray *DCSGetActiveDictionaries();
  NSArray *DCSCopyAvailableDictionaries();
  NSString *DCSDictionaryGetName(DCSDictionaryRef dictID);
  NSString *DCSDictionaryGetShortName(DCSDictionaryRef dictID);
}

QString Plugin::synopsis() const { return tr("<word>"); }

QString Plugin::defaultTrigger() const { return QStringLiteral("def "); }

void Plugin::handleTriggerQuery(TriggerQuery *query) const
{
    @autoreleasepool {
        DCSDictionaryRef dic = NULL;
        NSArray *dicts = DCSGetActiveDictionaries();
        for (NSObject *aDict in dicts) {
            dic = static_cast<DCSDictionaryRef>(aDict);

            NSString *word = query->string().toNSString();
            NSString *long_name = DCSDictionaryGetName((DCSDictionaryRef)aDict);
//            NSString *short_name = DCSDictionaryGetShortName((DCSDictionaryRef)aDict);
            NSString *result = (NSString*)DCSCopyTextDefinition(
                dic,
                (CFStringRef)word,
                DCSGetTermRangeInString(nil, (CFStringRef)word, 0)
            );
            if (result){
                auto text = QString::fromNSString(result);
                query->add(
                    StandardItem::make(
                        id(),
                        QString::fromNSString(long_name),
                        text,
                        {":dict"},
                        {
                            {
                                "open", tr("Open in dictionary"),
                                [s=query->string()](){ openUrl("dict://"+s); }
                            }
                        }
                    )
                );
            }
        }
    }
}
