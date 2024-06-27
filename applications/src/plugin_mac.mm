// Copyright (c) 2022-2024 Manuel Schneider

#include "plugin.h"
#include "ui_configwidget_mac.h"
#include <Cocoa/Cocoa.h>
#include <QCoreApplication>
#include <QDir>
#include <QWidget>
#include <albert/logging.h>
#include <albert/standarditem.h>
#include <albert/util.h>
#include <set>
ALBERT_LOGGING_CATEGORY("apps")
using namespace albert;
using namespace std;



static void addIndexItems(vector<IndexItem> &items, const QString &bundle_path, bool use_non_localized_name)
{
    set<QString> index_strings;
    @autoreleasepool {
        NSBundle *bundle = [NSBundle bundleWithPath:bundle_path.toNSString()];

        // INFO << "-------------------------------------";
        // INFO << "localized-display      " << QString::fromNSString([bundle.localizedInfoDictionary objectForKey:@"CFBundleDisplayName"]);
        // INFO << "localized              " << QString::fromNSString([bundle.localizedInfoDictionary objectForKey:(NSString *) kCFBundleNameKey]);
        // INFO << "unlocalized-display    " << QString::fromNSString([bundle.infoDictionary objectForKey:@"CFBundleDisplayName"]);
        // INFO << "unlocalized            " << QString::fromNSString([bundle.infoDictionary objectForKey:(NSString *) kCFBundleNameKey]);
        // INFO << "bundle path            " << bundle_path;
        // // WARN << "infoDictionary         " << QString::fromNSString([bundle.infoDictionary description]);
        // // WARN << "localizedInfoDictionar " << QString::fromNSString([bundle.localizedInfoDictionary description]);

        QString name;

        if (NSString *nss = [bundle.localizedInfoDictionary objectForKey:@"CFBundleDisplayName"];
                nss != nil)
        {
            auto s = QString::fromNSString(nss);
            index_strings.insert(s);
            if (name.isEmpty())
                name = s;
        }

        if (NSString *nss = [bundle.localizedInfoDictionary objectForKey:@"CFBundleName"];
                nss != nil)
        {
            auto s = QString::fromNSString(nss);
            index_strings.insert(s);
            if (name.isEmpty())
                name = s;
        }

        if (NSString *nss = [bundle.infoDictionary objectForKey:@"CFBundleDisplayName"];
                nss != nil && (name.isEmpty() || use_non_localized_name))
        {
            auto s = QString::fromNSString(nss);
            index_strings.insert(s);
            if (name.isEmpty())
                name = s;
        }

        if (NSString *nss = [bundle.infoDictionary objectForKey:@"CFBundleName"];
                nss != nil && (name.isEmpty() || use_non_localized_name))
        {
            auto s = QString::fromNSString(nss);
            index_strings.insert(s);
            if (name.isEmpty())
                name = s;
        }

        if (name.isEmpty())
            name = bundle_path.section("/", -1, -1).chopped(4);  // remove .app

        auto item = StandardItem::make(
            QString::fromNSString(bundle.bundleIdentifier),
            name,
            bundle_path,
            name,
            {QString("qfip:%1").arg(bundle_path)},
            {
                {
                    "launch", Plugin::tr("Launch app"),
                    [bundle_path]() { runDetachedProcess({"open", bundle_path}); }
                },
                {
                    "term", Plugin::tr("Open terminal here"),
                    [bundle_path]() { runTerminal({}, bundle_path); }
                }
            }
        );

        for (const auto &s : index_strings)
            items.emplace_back(item, s);
    }
}

class Plugin::Private {};  // Not used on macos

Plugin::Plugin() : d(make_unique<Private>())
{
    auto s = settings();
    commonInitialize(s);

    indexer_.parallel = [this](const bool &abort)
    {
        QStringList apps;
        apps << "/System/Library/CoreServices/Finder.app";
        for (const auto &path : appDirectories())
            for (const auto &fi : QDir(path).entryInfoList({"*.app"}))
                apps << fi.absoluteFilePath();

        vector<IndexItem> ii;
        for (const auto & app : apps)
            if (abort)
                return ii;
            else
                addIndexItems(ii, app, use_non_localized_name());

        INFO << QString("Indexed %1 apps.").arg(apps.size());

        return ii;
    };

    indexer_.finish = [this](auto &&r) { setIndexItems(::move(r)); };

}

Plugin::~Plugin() = default;

void Plugin::updateIndexItems() { indexer_.run(); }

QWidget *Plugin::buildConfigWidget()
{
    auto *w = new QWidget;
    Ui::ConfigWidget ui;
    ui.setupUi(w);

    ALBERT_PROPERTY_CONNECT_CHECKBOX(this, use_non_localized_name, ui.checkBox_useNonLocalizedName);

    return w;
}

QStringList Plugin::appDirectories()
{
    return {
        "/Applications",
        "/Applications/Utilities",
        "/System/Applications",
        "/System/Applications/Utilities",
        "/System/Library/CoreServices/Applications",
        "/System/Library/CoreServices/Finder.app/Contents/Applications"
    };
}


































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
