// Copyright (c) 2022-2024 Manuel Schneider
//
// See also:
// - https://github.com/NSHipster/DictionaryKit
// - https://discussions.apple.com/thread/6616776?sortBy=rank
//

#include "plugin.h"
#include <QUrl>
#include <CoreServices/CoreServices.h>
#include <Foundation/Foundation.h>
#include <albert/logging.h>
#include <albert/standarditem.h>
#include <albert/util.h>
ALBERT_LOGGING_CATEGORY("dictionary")
using namespace albert;
using namespace std;
#if  ! __has_feature(objc_arc)
#error This file must be compiled with ARC.
#endif

namespace
{

const QStringList icon_urls = {"qfip:/System/Applications/Dictionary.app"};

typedef NS_ENUM(NSInteger, DictionaryRecordVersion) {
    HTML = 0,
    HTMLWithAppCSS = 1,
    HTMLWithPopoverCSS = 2,
    Text = 3,
};

typedef NS_ENUM(NSInteger, SearchOptions) {
    Exact = 0,
    Prefix = 1,
    Tolerant = 2
};

static function<void()> makeSearchFunc(const QString &term)
{
    return [s = term]
    {
        // QUrl refuses to parse addressboolk urls. Use platform open.
        // https://bugreports.qt.io/browse/QTBUG-129496
        auto url = QString::fromUtf8(QUrl::toPercentEncoding(s));
        url = QString("dict://%1").arg(url);
        albert::runDetachedProcess({"open", url});
    };
}

static inline QString n2q(const NSString * nss) { return QString::fromNSString(nss); }

}

extern "C"
{
CFArrayRef DCSGetActiveDictionaries();
CFArrayRef DCSCopyAvailableDictionaries();
CFStringRef DCSDictionaryGetName(DCSDictionaryRef dictionary);
CFStringRef DCSDictionaryGetShortName(DCSDictionaryRef dictionary);
DCSDictionaryRef DCSDictionaryCreate(CFURLRef url);
CFArrayRef DCSCopyRecordsForSearchString(DCSDictionaryRef dictionary, CFStringRef string,
                                         long search_options, long max_records);
DCSDictionaryRef DCSRecordGetSubDictionary(CFTypeRef record);
CFDictionaryRef DCSCopyDefinitionMarkup(DCSDictionaryRef dictionary, CFStringRef record);
CFStringRef DCSRecordGetHeadword(CFTypeRef record);
CFStringRef DCSRecordGetRawHeadword(CFTypeRef record);
CFStringRef DCSRecordGetTitle(CFTypeRef record);
CFStringRef DCSRecordGetString(CFTypeRef record);
CFStringRef DCSRecordGetAnchor(CFTypeRef record);
CFStringRef DCSRecordGetAssociatedObj(CFTypeRef record);
CFStringRef DCSRecordCopyData(CFTypeRef record, long version);
CFStringRef DCSRecordCopyDataURL(CFTypeRef record);
}

Plugin::Plugin():
    tr_ {
        .dict = tr("Dictionary"),
        .lookup = tr("Lookup in dictionary"),
        .lookup_arg = tr("Lookup '%1' in dictionary")
    }
{}

QString Plugin::defaultTrigger() const { return QStringLiteral("def "); }

void Plugin::handleTriggerQuery(Query *query)
{
    auto &&q_query = query->string();
    auto ns_query = query->string().toNSString();
    auto cf_query = (__bridge CFStringRef)ns_query;

    for (NSObject *ns_object in (__bridge NSArray *)DCSGetActiveDictionaries())
    {
        @autoreleasepool
        {
            DCSDictionaryRef dict = (__bridge DCSDictionaryRef)ns_object;

            // auto s_name = n2q((__bridge NSString *)DCSDictionaryGetShortName(dict));
            auto dict_name = n2q((__bridge NSString *)DCSDictionaryGetName(dict));

            // CFRange range = DCSGetTermRangeInString(dict, cf_query, 0);
            // if (range.location == kCFNotFound
            //     || range.length != q_query.length())  // full matches only
            // {
            //     CRIT << range.location << range.length;
            //     continue;
            // }
            // auto definition = n2q((__bridge_transfer NSString *)
            //                       DCSCopyTextDefinition(dict, cf_query, range));


            if (auto *records = (__bridge_transfer NSArray *)
                DCSCopyRecordsForSearchString(dict, cf_query, Exact, NULL); records)
            {
                for (::id record in records)
                {
                    auto r = (__bridge CFTypeRef)record;

                    auto headword     = n2q((__bridge NSString *)DCSRecordGetHeadword(r));
                    // auto raw_headword = n2q((__bridge NSString *)DCSRecordGetRawHeadword(r));
                    // auto title        = n2q((__bridge NSString *)DCSRecordGetTitle(r));
                    // auto string       = n2q((__bridge NSString *)DCSRecordGetString(r));
                    // auto anchor       = n2q((__bridge NSString *)DCSRecordGetAnchor(r));

                    // auto data_url = n2q((__bridge_transfer NSString *)DCSRecordCopyDataURL(r));

                    // auto html = n2q((__bridge_transfer NSString *)
                    //                 DCSRecordCopyData(r, HTMLWithPopoverCSS));

                    auto text = n2q((__bridge_transfer NSString *)DCSRecordCopyData(r, Text));

                    // WARN << "---" << record;
                    // INFO << "headword" << headword;
                    // INFO << "raw_headword" << raw_headword;
                    // INFO << "string" << string;
                    // INFO << "title" << title;
                    // INFO << "anchor" << anchor;
                    // // INFO << "html" << html;
                    // // INFO << "text" << text;

                    // INFO << "data_url" << data_url;
                    // // INFO << "data" << data;

                    vector<Action> actions;

                    actions.emplace_back("search_hw",
                                         tr_.lookup_arg.arg(headword),
                                         makeSearchFunc(headword));

                    if (QString::compare(q_query, headword, Qt::CaseInsensitive) != 0)
                        actions.emplace_back("search_q",
                                             tr_.lookup_arg.arg(q_query),
                                             makeSearchFunc(q_query));

                    query->add(StandardItem::make(
                        id(), dict_name, text,icon_urls, ::move(actions)));
                }
            }
        }
    }
}

vector<shared_ptr<Item> > Plugin::fallbacks(const QString &s) const
{
    return {
        StandardItem::make(
            "search",
            tr_.dict,
            tr_.lookup_arg.arg(s),
            icon_urls,
            {{"search", tr_.lookup, makeSearchFunc(s) }}
        )
    };
}
