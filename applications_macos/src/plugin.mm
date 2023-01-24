// Copyright (c) 2022-2023 Manuel Schneider

#include <Cocoa/Cocoa.h>
#include "plugin.h"
ALBERT_LOGGING
using namespace std;
using namespace albert;

static const QStringList watched_dirs = {"Applications", "/Applications"};

//static void printNSMetadataItem(NSMetadataItem *item)
//{
//  INFO << QString::fromNSString([item valueForAttribute:NSMetadataItemPathKey]);
//  for (NSString * attr in [item attributes])
//    NSLog(@"%@: %@", attr, [item valueForAttribute:attr]);  // Objects returned getting this into qt is too much
//}

//static void printBundleInfo(NSBundle *bundle)
//{
//  auto printNSString = [](NSString *s){ INFO << QString::fromNSString(s); };
//  INFO << "------ BundleInfo ------";
//  printNSString(bundle.bundlePath);
//  printNSString(bundle.bundleIdentifier);
//  printNSString(bundle.executablePath);
//  printNSString([bundle objectForInfoDictionaryKey:(NSString*)kCFBundleNameKey]);
//  printNSString([bundle objectForInfoDictionaryKey:(NSString*)kCFBundleVersionKey]);
//  printNSString([bundle objectForInfoDictionaryKey:@"CFBundleDisplayName"]);
//}

static QStringList getAppBundlePaths()
{
    QStringList results;
    @autoreleasepool {
        // Run a spotlight query
        NSMetadataQuery *query = [[NSMetadataQuery alloc] init];
        [query setSearchScopes:[NSArray arrayWithObjects:@"Applications", @"/Applications", @"/System/Applications", @"/System/Library/CoreServices/Applicatis", nil]];
        [query setPredicate:[NSPredicate predicateWithFormat:@"kMDItemContentType == 'com.apple.application-bundle'"]]; //  || kMDItemContentType == 'com.apple.systempreference.prefpane'"]];
        if ([query startQuery]) {
            while ([query isGathering])
                [[NSRunLoop currentRunLoop] runUntilDate:[NSDate dateWithTimeIntervalSinceNow:0.1]];
            [query stopQuery];
        }

        for (NSMetadataItem *item in query.results) {
            results << QString::fromNSString([item valueForAttribute:NSMetadataItemPathKey]);
        }
    }

    results << "/System/Library/CoreServices/Finder.app";
    results << "/System/Library/CoreServices/Finder.app/Contents/Applications/AirDrop.app";
    results << "/System/Library/CoreServices/Finder.app/Contents/Applications/Network.app";
    results << "/System/Library/CoreServices/Finder.app/Contents/Applications/Computer.app";
    results << "/System/Library/CoreServices/Finder.app/Contents/Applications/iCloud Drive.app";
    results << "/System/Library/CoreServices/Finder.app/Contents/Applications/Recents.app";

    return results;
}

static shared_ptr<Item> createAppItem(const QString &bundle_path)
{
    @autoreleasepool {
        NSBundle *bundle = [NSBundle bundleWithPath:bundle_path.toNSString()];

//    printNSMetadataItem(item);
//    printBundleInfo(bundle);

        QString name;
        if (NSString *nss = [bundle objectForInfoDictionaryKey:@"CFBundleDisplayName"]; nss != nil)
            name = QString::fromNSString(nss);
        else if (nss = [bundle objectForInfoDictionaryKey:(NSString *) kCFBundleNameKey]; nss != nil)
            name = QString::fromNSString(nss);
        else {
            name = bundle_path.section("/", -1, -1);
        }

        if (name.endsWith(".app"))
            name.chop(4);

        return StandardItem::make(
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
    }
}

Plugin::Plugin()
{
    connect(&fs_watcher_, &QFileSystemWatcher::directoryChanged, this, [this]() { indexer.run(); });
    fs_watcher_.addPaths(watched_dirs);

    indexer.parallel = [](const bool &abort) {
        vector<IndexItem> results;
        for (const QString &bundle_path: getAppBundlePaths()) {
            if (abort) return results;
            auto app_item = createAppItem(bundle_path);
            results.emplace_back(app_item, app_item->text());
        }
        return results;
    };
    indexer.finish = [this](vector<IndexItem> &&result) {
        setIndexItems(::move(result));
    };
}

void Plugin::updateIndexItems()
{
    indexer.run();
}


// // older snippets
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
