// Copyright (c) 2022-2024 Manuel Schneider

#include "application.h"
#include <Cocoa/Cocoa.h>
#include <QFileInfo>
#include <albert/logging.h>
#include <albert/albert.h>
using namespace albert;
using namespace std;
#if  ! __has_feature(objc_arc)
#error This file must be compiled with ARC.
#endif

void printBundleInfo(QString path, NSBundle *bundle)
{
    INFO << "-------------------------------------";
    INFO << path;
    INFO << "bundleIdentifier       " << QString::fromNSString(bundle.bundleIdentifier);
    INFO << "localized-display      " << QString::fromNSString([bundle.localizedInfoDictionary objectForKey:@"CFBundleDisplayName"]);
    INFO << "localized              " << QString::fromNSString([bundle.localizedInfoDictionary objectForKey:(NSString *) kCFBundleNameKey]);
    INFO << "unlocalized-display    " << QString::fromNSString([bundle.infoDictionary objectForKey:@"CFBundleDisplayName"]);
    INFO << "unlocalized            " << QString::fromNSString([bundle.infoDictionary objectForKey:(NSString *) kCFBundleNameKey]);
    // WARN << "infoDictionary         " << QString::fromNSString([bundle.infoDictionary description]);
    // WARN << "localizedInfoDictionar " << QString::fromNSString([bundle.localizedInfoDictionary description]);

    // NSURL *bundleURL = [bundle bundleURL];
    // NSWorkspace *workspace = [NSWorkspace sharedWorkspace];

    NSArray *documentTypes = [bundle.infoDictionary objectForKey:@"CFBundleDocumentTypes"];
    if (documentTypes != nil)
    {
        for (NSDictionary *documentType in documentTypes)
        {
            // NSString *typeName = [documentType objectForKey:@"CFBundleTypeName"];
            NSString *typeRole = [documentType objectForKey:@"CFBundleTypeRole"];
            NSString *handlerRank = [documentType objectForKey:@"LSHandlerRank"];
            NSArray *contentTypes = [documentType objectForKey:@"LSItemContentTypes"];
            NSArray *fileExtensions = [documentType objectForKey:@"CFBundleTypeExtensions"];
            NSArray *mimeTypes = [documentType objectForKey:@"CFBundleTypeMIMETypes"];
            // NSString *iconFile = [documentType objectForKey:@"CFBundleTypeIconFile"];

            // NSLog(@"Document Type Name: %@", typeName);
            // NSLog(@"Document Type Role: %@", typeRole);
            // NSLog(@"Content Types: %@", contentTypes);
            // NSLog(@"File Extensions: %@", fileExtensions);
            // NSLog(@"MIME Types: %@", mimeTypes);
            // NSLog(@"Icon File: %@", iconFile);

            if (typeRole != nil)
                WARN << "typeRole" << QString::fromNSString(typeRole);

            if (handlerRank != nil)
                WARN << "handlerRank" << QString::fromNSString(handlerRank);

            if (contentTypes != nil)
                for (NSString *contentType in contentTypes)
                    WARN << "contentType" << QString::fromNSString(contentType);

            if (fileExtensions != nil)
                for (NSString *fileExtension in fileExtensions)
                    WARN << "fileExtension" << QString::fromNSString(fileExtension);

            if (mimeTypes != nil)
                for (NSString *mimeType in mimeTypes)
                    WARN << "mimeType" << QString::fromNSString(mimeType);
        }
    }
    // NSError *error = nil;
    // NSArray *types = [workspace typeIdentifiersForBundleAtURL:bundleURL error:&error];
}

Application::Application(const QString &path, bool use_non_localized_name)
{
    path_ = QFileInfo(path).absoluteFilePath();

    @autoreleasepool {
        NSBundle *bundle = [NSBundle bundleWithPath:path_.toNSString()];
        // printBundleInfo(path_, bundle);

        id_ = QString::fromNSString(bundle.bundleIdentifier);
        if (id_.isEmpty())
            throw runtime_error(format("No bundle identifier for {}.", path_.toStdString()));

        NSString *nss;
        nss = [bundle.localizedInfoDictionary objectForKey:@"CFBundleDisplayName"];
        if (nss == nil)
        {
            nss = [bundle.localizedInfoDictionary objectForKey:@"CFBundleName"];
            if (nss != nil)
                if (auto name = QString::fromNSString(nss); !names_.contains(name))
                    names_ << QString::fromNSString(nss);
        }
        else
            names_ << QString::fromNSString(nss);

        if (use_non_localized_name)
        {
            nss = [bundle.infoDictionary objectForKey:@"CFBundleDisplayName"];
            if (nss == nil)
            {
                nss = [bundle.infoDictionary objectForKey:@"CFBundleName"];
                if (nss != nil)
                    if (auto name = QString::fromNSString(nss); !names_.contains(name))
                        names_ << QString::fromNSString(nss);
            }
            else
            {
                if (auto name = QString::fromNSString(nss); !names_.contains(name))
                    names_ << name;
            }
        }

        if (names_.isEmpty() || use_non_localized_name)
            if (auto name = path_.section("/", -1).chopped(4); !names_.contains(name))// remove .app
                names_ << name;

        // Remove soft hyphens
        for(auto &name : names_)
            name.remove(QChar(0x00AD));
    }
}

QString Application::subtext() const { return path_; }

QStringList Application::iconUrls() const { return {QString("qfip:%1").arg(path_)}; }

void Application::launch() const { runDetachedProcess({"open", path_}); }
