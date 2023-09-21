// Copyright (c) 2022-2023 Manuel Schneider

#include "albert/albert.h"
#include "albert/logging.h"
#include "albert/extension/queryhandler/standarditem.h"
#include "plugin.h"
#include <Cocoa/Cocoa.h>
#include <QDir>
ALBERT_LOGGING_CATEGORY("apps")
using namespace albert;
using namespace std;

class Plugin::Private
{
public:
    NSMetadataQuery *query;

    static IndexItem createAppIndexItem(const QString &bundle_path, const QString &display_name = {})
    {
        @autoreleasepool {
            NSBundle *bundle = [NSBundle bundleWithPath:bundle_path.toNSString()];
            QString name;
            if (!display_name.isNull())
                name = display_name;
            else if (NSString *nss = [bundle objectForInfoDictionaryKey:@"CFBundleDisplayName"]; nss != nil)
                name = QString::fromNSString(nss);
            else if (nss = [bundle objectForInfoDictionaryKey:(NSString *) kCFBundleNameKey]; nss != nil)
                name = QString::fromNSString(nss);
            else
                name = bundle_path.section("/", -1, -1);

            if (name.endsWith(".app"))
                name.chop(4);

            auto item = StandardItem::make(
                QString::fromNSString(bundle.bundleIdentifier),
                name,
                bundle_path,
                {QString("qfip:%1").arg(bundle_path)},
                {
                    {
                        "launch",
                        "Launch app",
                        [bundle_path]() { runDetachedProcess({"open", bundle_path}); }
                    }
                }
                );

            return IndexItem(::move(item), name);
        }
    }

    vector<IndexItem> getAppsIndexItems()
    {
        [query disableUpdates];

        vector<IndexItem> results;

        @autoreleasepool {
            // Gather results and extract app data
            for (NSMetadataItem *item in query.results) {
                //printNSMetadataItem(item);
                auto bundle_path = QString::fromNSString([item valueForAttribute:NSMetadataItemPathKey]);
                auto display_name = QString::fromNSString([item valueForAttribute:@"kMDItemDisplayName"]);
                results.emplace_back(createAppIndexItem(bundle_path, display_name));
            }
        }

        results.emplace_back(createAppIndexItem("/System/Library/CoreServices/Finder.app"));
        for (const auto &entry : QDir("/System/Library/CoreServices/Finder.app/Contents/Applications/").entryInfoList({"*.app"}))
            results.emplace_back(createAppIndexItem(entry.absoluteFilePath()));

        for (const auto &entry : QDir("/System/Library/PreferencePanes/").entryInfoList({"*.prefPane"}))
            results.emplace_back(createAppIndexItem(entry.absoluteFilePath()));

        [query enableUpdates];

        INFO << QString("Indexed %1 apps.").arg(results.size());

        return results;
    }
};


Plugin::Plugin() : d(new Private)
{
    d->query = [[NSMetadataQuery alloc] init];

    [d->query setSearchScopes:[NSArray arrayWithObjects:QDir::homePath().toNSString(),
                                                        @"/Applications",
                                                        @"/System/Applications",
                                                        @"/System/Library/CoreServices/Applications",
                                                        nil]];

    [d->query setPredicate:[NSPredicate predicateWithFormat:@"kMDItemContentType == 'com.apple.application-bundle'"]];

    if (![d->query startQuery])
        WARN << "Could not start NSMetadataQuery.";

    for (auto notification : {NSMetadataQueryDidFinishGatheringNotification,
                              NSMetadataQueryDidUpdateNotification})
        [[NSNotificationCenter defaultCenter]
            addObserverForName:notification
                        object:nil
                         queue:[NSOperationQueue new]
                    usingBlock:^(NSNotification __strong *) {
                        updateIndexItems();
                    }
        ];
}

Plugin::~Plugin()
{
    [d->query stopQuery];
    [d->query release];
}

QString Plugin::defaultTrigger() const { return QStringLiteral("apps "); }

void Plugin::updateIndexItems() { setIndexItems(d->getAppsIndexItems()); }








// // older snippets



//static void printNSMetadataItem(NSMetadataItem *item)
//{
//  INFO << QString::fromNSString([item valueForAttribute:NSMetadataItemPathKey]);
//  for (NSString * attr in [item attributes])
//    NSLog(@"%@: %@", attr, [item valueForAttribute:attr]);  // Objects returned getting this into qt is too much
//}

//#define currentLanguageBundle [NSBundle bundleWithPath:[[NSBundle mainBundle] pathForResource:[[NSLocale preferredLanguages] objectAtIndex:0] ofType:@"lproj"]]

//static void printBundleInfo(NSBundle *bundle)
//{
//  auto ns2qs = [](NSString *s){ return QString::fromNSString(s); };
//  auto nsobj2nss = [](auto *d){ return [NSString stringWithFormat:@"%@", d]; };

//  [[NSUserDefaults standardUserDefaults]
//      setObject:[NSArray arrayWithObject:@"de"]
//         forKey:@"AppleLanguages"];

//  auto *locale = [NSLocale currentLocale];
//  INFO << "locale languageCode" << nsobj2nss(locale.languageCode);
//  INFO << "locale localizedStringForLanguageCode" << nsobj2nss([locale localizedStringForLanguageCode:locale.languageCode]);

//  auto *main = [NSBundle mainBundle];
//  INFO << "main bundlePath" << ns2qs(main.bundlePath);
//  INFO << "main preferredLocalizations" << ns2qs(nsobj2nss(main.preferredLocalizations));

//  INFO << "------ BundleInfo ------";

//  NSString *path = [bundle pathForResource:@"de" ofType:@"lproj" ];
//  INFO << "lpath" << ns2qs(path);


//  auto *lb = [[NSBundle bundleWithPath:path] retain];
//  INFO << "lb localizedInfoDictionary" << ns2qs(nsobj2nss(lb.infoDictionary));
//  NSLog(@"localizedStringForKey: %@", [lb localizedStringForKey: (@"CFBundleName") value: nil table: nil]);

//  auto *s = NSLocalizedStringFromTableInBundle(@"CFBundleDisplayName", nil, currentLanguageBundle, @"");
//  INFO << "NSLocalizedStringFromTableInBundle" << ns2qs(s);


//  NSFileManager *fileManager = [[NSFileManager alloc] init];
//  INFO << "displayNameAtPath" << [fileManager displayNameAtPath:bundle.bundlePath];

//  [[NSUserDefaults standardUserDefaults] setObject:[NSArray arrayWithObjects:@"de", nil] forKey:@"AppleLanguages"];
//  [[NSUserDefaults standardUserDefaults] synchronize]; //to make the change immediate

//  INFO << "bundlePath" << ns2qs(bundle.bundlePath);
//  INFO << "bundleIdentifier" << ns2qs(bundle.bundleIdentifier);
//  INFO << "executablePath" << ns2qs(bundle.executablePath);
//  INFO << "localizations" << ns2qs(nsobj2nss(bundle.localizations));
//  INFO << "developmentLocalization" << ns2qs(nsobj2nss(bundle.developmentLocalization));
//  INFO << "preferredLocalizations" << ns2qs(nsobj2nss(bundle.preferredLocalizations));
////  main.preferredLocalizations = [NSArray arrayWithObjects:@"de", nil]
//  INFO << "localizedInfoDictionary" << ns2qs(nsobj2nss(bundle.localizedInfoDictionary));
////  INFO << "infoDictionary" << ns2qs(nsobj2nss(bundle.infoDictionary));

//  // Get the localized name using the specified key and bundle
////  NSString *localizedAppName = NSLocalizedStringFromTableInBundle(@"CFBundleDisplayName", nil, bundle, nil);
//  NSString *localizedAppName = [bundle localizedStringForKey: (@"CFBundleDisplayName") value: nil table: nil];
//  NSLog(@"Localized App Name: %@", localizedAppName);

//  NSLog(@"localizedStringForKey: %@", [bundle localizedStringForKey: (@"CFBundleDisplayName") value: nil table: nil]);
//}



// NSString *ns_bundle_identifier = [result valueForAttribute:(NSString *)kMDItemCFBundleIdentifier];
// auto name = nsstrtoqstr([result valueForAttribute:(NSString *)kMDItemDisplayName]);
// NSURL *ns_url = [[NSWorkspace sharedWorkspace] URLForApplicationWithBundleIdentifier:ns_bundle_identifier];
// auto url = nsstrtoqstr([[ns_url  absoluteString ] stringByRemovingPercentEncoding]);
//    for (NSMetadataItem * result in [query results]) {
//      NSString *ns_bundle_identifier = [result valueForAttribute:(NSString *)kMDItemCFBundleIdentifier];
//            auto name = QString::fromNSString([result valueForAttribute:(NSString *)kMDItemDisplayName]);
//            NSString *ns_item_desc = [result valueForAttribute:(NSString *)kMDItemDescription];
//            if (ns_item_desc == nil)
//                WARN << "kMDItemDescription" <<  QString::fromNSString((NSString *)ns_item_desc);

//    //        NSWorkspaceOpenConfiguration* configuration = [NSWorkspaceOpenConfiguration new];
//    //        [[NSWorkspace sharedWorkspace] openApplicationAtURL: [NSURL fileURLWithPath: path]
//    //                                              configuration: configuration
//    //                                                            error: &error];

//  if (![[NSWorkspace sharedWorkspace]
//        launchAppWithBundleIdentifier:qstrtonsstr(identifier)
//                              options:NSWorkspaceLaunchDefault
//       additionalEventParamDescriptor:NULL
//                     launchIdentifier:NULL]) {
