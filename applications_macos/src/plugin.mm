// Copyright (c) 2022 Manuel Schneider

#include <Cocoa/Cocoa.h>
#include "plugin.h"
ALBERT_LOGGING
using namespace std;
using albert::StandardItem;

static const QStringList app_dirs = {
    "Applications",
    "/Applications",
    "/System/Applications",
    "/System/Library/CoreServices/Applications"
};

NSString * qstrtonsstr (const QString &string) { return [NSString stringWithUTF8String: string.toUtf8().constData()]; }

QString nsstrtoqstr (NSString *string) { return QString([string UTF8String]); }

Plugin::Plugin()
{
    connect(&fs_watcher_, &QFileSystemWatcher::directoryChanged, this, [this](){ indexer.run(); });
    fs_watcher_.addPaths(QStringList(app_dirs));

    indexer.parallel = &Plugin::indexApps;
    indexer.finish = [this](vector<shared_ptr<StandardItem>> &&result){ apps = ::move(result); updateIndex(); };
    indexer.run();
}

vector<shared_ptr<albert::StandardItem>> Plugin::indexApps(const bool &abort)
{
    std::vector<std::shared_ptr<albert::StandardItem>> results;
    @autoreleasepool {
        NSMetadataQuery * query = [[NSMetadataQuery alloc] init];
        [query setSearchScopes: [NSArray arrayWithObjects: @"Applications", @"/Applications", @"/System/Applications", @"/System/Library/CoreServices/Applications", nil]];
        [query setPredicate:[NSPredicate predicateWithFormat:@"kMDItemContentType == 'com.apple.application-bundle' || kMDItemContentType == 'com.apple.systempreference.prefpane'"]];
        if ([query startQuery]){
            while ([query isGathering])
                [[NSRunLoop currentRunLoop] runUntilDate:[NSDate dateWithTimeIntervalSinceNow:0.1]];
            [query stopQuery];
        }

        for (NSMetadataItem * result in [query results]) {

            NSString *ns_bundle_identifier = [result valueForAttribute:(NSString *)kMDItemCFBundleIdentifier];
            if (ns_bundle_identifier == nil){
                WARN << "(ns_bundle_identifier == nil)";
                continue;
            }
            auto identifier = nsstrtoqstr(ns_bundle_identifier);
            auto name = nsstrtoqstr([result valueForAttribute:(NSString *)kMDItemDisplayName]);
            name.chop(4); // rem .app
            NSURL *ns_url = [[NSWorkspace sharedWorkspace] URLForApplicationWithBundleIdentifier:ns_bundle_identifier];
            auto url = nsstrtoqstr([[ns_url  absoluteString ] stringByRemovingPercentEncoding]);
            if (url.isEmpty())
                continue;
            auto surl = QUrl(url).toLocalFile();

//            DEBG << name << identifier << url << surl;

            results.emplace_back(StandardItem::make(
                    identifier,
                    name,
                    surl,
                    {QString("qfip:%1").arg(surl)},
                    {
                            {"launch", "Launch app", [identifier](){
                                if (![[NSWorkspace sharedWorkspace]
                                        launchAppWithBundleIdentifier:qstrtonsstr(identifier)
                                                              options:NSWorkspaceLaunchDefault
                                       additionalEventParamDescriptor:NULL
                                                     launchIdentifier:NULL]) {
                                    qWarning() << "Launching app failed!";
                                }
                            }}
                    }
            ));
        }
    }
    return results;
}

vector<albert::IndexItem> Plugin::indexItems() const
{
    std::vector<albert::IndexItem> items;
    for (const auto &app : apps)
        items.emplace_back(app, app->text());
    return items;
}








//void Plugin::scanApps()
//{
//    apps.clear();
//    for (const auto *app_dir : app_dirs){
//        QDir dir(app_dir);
//        for (const QFileInfo &file_info : dir.entryInfoList({"*.app"})){
//            auto abs_path = file_info.canonicalFilePath();
//            auto *bundle = getBundleForPath(qstrtonsstr(abs_path));
//
//
//            CRIT << nsstrtoqstr([bundle objectForInfoDictionaryKey:@"CFBundleName"]);
//            CRIT << nsstrtoqstr([[bundle localizedInfoDictionary] objectForKey:@"CFBundleName"]);
//            CRIT << nsstrtoqstr([bundle objectForInfoDictionaryKey:@"CFBundleDisplayName"]);
//            CRIT << nsstrtoqstr([[bundle localizedInfoDictionary] objectForKey:@"CFBundleDisplayName"]);
//            CRIT << nsstrtoqstr([bundle objectForInfoDictionaryKey:(NSString*)kCFBundleNameKey]);
//
//            INFO << nsstrtoqstr(bundle.bundleIdentifier);
//            INFO << nsstrtoqstr(bundle.bundlePath);
//
////            CRIT << "bundle.localizations";
////            for (NSString *nss in bundle.localizations)
////                INFO << nsstrtoqstr(nss);
//
//            NSString *language = NSBundle.mainBundle.preferredLocalizations.firstObject;
//            NSLocale *locale = NSLocale.currentLocale;
//            NSString *countryCode = [locale objectForKey:NSLocaleCountryCode];
//            INFO << nsstrtoqstr(countryCode);
//            INFO << nsstrtoqstr(NSLocale.preferredLanguages[0]);
//
//
//
//
//            auto *local_ns_name = [bundle objectForInfoDictionaryKey:(NSString*)kCFBundleNameKey];
//            auto localized_name = nsstrtoqstr(local_ns_name);
//
//            apps.emplace_back(make_shared<albert::StandardItem>(
//                    file_info.fileName(),
//                    localized_name,
//                    abs_path,
//                    QStringList{QString("qfip:%1").arg(abs_path)},
//                    albert::Actions {
//                            {localized_name, localized_name,
//                             [abs_path](){ albert::runDetachedProcess({"open", abs_path}); }}
//                    },
//                    localized_name
//            ));
//        }
//    }
//    INFO << QString("Indexed %1 applications.").arg(apps.size());
//}


//
//NSBundle *getBundleForPath(NSString *fullPath)
//{
//    return [NSBundle bundleWithPath:fullPath];
//}
//
//NSString * getLocalizedDisplayNameforBundle(NSBundle *bundle)
//{
//    auto *i = [bundle localizedInfoDictionary];
//    if (i==nil)
//        WARN << "localizedInfoDictionary is nil";
//
//    auto p = [i objectForKey:@"CFBundleDisplayName"];
//    if (p==nil)
//        WARN << "CFBundleDisplayName is nil";
//    return p;
//}

//void launchBundle(NSString *bundle_identifier)
//{
//    //    @autoreleasepool {
//    //        NSBundle * bundle = getBundleForPath(qstrtonsstr(file_path));
//    //        NSWorkspaceOpenConfiguration* configuration = [NSWorkspaceOpenConfiguration new];
//    //        [[NSWorkspace sharedWorkspace] openApplicationAtURL: [NSURL fileURLWithPath: path]
//    //                                              configuration: configuration
//    //                                                            error: &error];

//    //    }
//    @autoreleasepool {

//        if (![[NSWorkspace sharedWorkspace]
//               launchAppWithBundleIdentifier:bundle_identifier
//                                     options:NSWorkspaceLaunchDefault
//              additionalEventParamDescriptor:NULL
//                            launchIdentifier:NULL]) {
//              WARN << "Launching app failed!";
//        }
//    }
//}
