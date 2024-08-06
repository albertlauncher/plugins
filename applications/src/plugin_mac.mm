// Copyright (c) 2022-2024 Manuel Schneider

#include "applications.h"
#include "plugin.h"
#include "ui_configwidget_mac.h"
#include <Cocoa/Cocoa.h>
#include <QCoreApplication>
#include <QDir>
#include <QWidget>
#include <albert/logging.h>
#include <albert/standarditem.h>
#include <albert/util.h>
#include <pwd.h>
#include <set>
#include <unistd.h>
using namespace albert;
using namespace std;

#import <Foundation/Foundation.h>
#import <AppKit/AppKit.h>
#include <QMessageBox>

static Plugin* plugin = nullptr;


static void printBundleInfo(QString path, NSBundle *bundle)
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
            NSString *typeName = [documentType objectForKey:@"CFBundleTypeName"];
            NSString *typeRole = [documentType objectForKey:@"CFBundleTypeRole"];
            NSString *handlerRank = [documentType objectForKey:@"LSHandlerRank"];
            NSArray *contentTypes = [documentType objectForKey:@"LSItemContentTypes"];
            NSArray *fileExtensions = [documentType objectForKey:@"CFBundleTypeExtensions"];
            NSArray *mimeTypes = [documentType objectForKey:@"CFBundleTypeMIMETypes"];
            NSString *iconFile = [documentType objectForKey:@"CFBundleTypeIconFile"];

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


class Application : public applications::Application,
                    public albert::Item
{
public:
    Application(const QString &p, bool use_non_localized_name)
        : bundle_path_(QFileInfo(p).absoluteFilePath())
    {


        @autoreleasepool {
            NSBundle *bundle = [NSBundle bundleWithPath:bundle_path_.toNSString()];
            printBundleInfo(bundle_path_, bundle);

            id_ = QString::fromNSString(bundle.bundleIdentifier);
            if (id_.isEmpty())
                throw runtime_error(format("No bundle identifier for {}.", bundle_path_.toStdString()));

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
                if (auto name = bundle_path_.section("/", -1, -1).chopped(4); !names_.contains(name))// remove .app
                    names_ << QString::fromNSString(nss);
        }
    }

    QString path() const override { return bundle_path_; }

    QString id() const override { return id_; }

    QString name() const override { return names_[0]; }

    QStringList names() const { return names_; }

    QString text() const override { return names_[0]; }

    QString inputActionText() const override { return names_[0]; }

    QString subtext() const override { return bundle_path_; }

    QStringList iconUrls() const override { return {QString("qfip:%1").arg(bundle_path_)}; }

    vector<Action> actions() const override
    {
        vector<Action> actions;

        actions.emplace_back("launch", Plugin::tr("Launch app"),
                             [this]{ launch(); });

        // actions.emplace_back("term", Plugin::tr("Open terminal here"),
        //                      [this]{ plugin->runpluginuser_launch(); });

        return actions;
    }

    void launch(const QString &working_dir = {}) const override
    {
        runDetachedProcess({"open", bundle_path_}, working_dir);
    }

    void launchWithUrls(QStringList urls = {},  const QString &working_dir = {}) const override
    {
        Q_UNUSED(urls);
        Q_UNUSED(working_dir);
        qFatal("Not implemented");
    }

private:
    QString bundle_path_;
    QString id_;
    QStringList names_;

};


class Terminal : public applications::Terminal
{
    shared_ptr<Application> app_;
public:
    Terminal(const shared_ptr<Application> &app) : app_(app) {}
    QString id() const override { return app_->id(); }
    QString name() const override { return app_->name(); }
    QString path() const override { return app_->path(); }

    void launch(const QString &working_dir) const override {
        app_->launch(working_dir);
    }

    void launchWithUrls(QStringList urls, const QString &working_dir) const override
    {
        app_->launchWithUrls(urls, working_dir);
    }
};

class AppleScriptLaunchableTerminal : public Terminal
{
    const char *apple_script_;
public:

    /// \note the apple script must contain the placeholder %1 for the command line to run
    AppleScriptLaunchableTerminal(const shared_ptr<Application> &app, const char* apple_script)
        : Terminal(app), apple_script_(apple_script) {}



    void launch(const QString &working_dir) const override
    {
        launchWithScript("exec $SHELL", working_dir);
    }

    void launchWithScript(QString script, const QString &working_dir) const override
    {
        if (!working_dir.isEmpty())
            script = QString("cd \"%1\"; ").arg(working_dir) + script;

        if (passwd *pwd = getpwuid(geteuid()); pwd == nullptr)
            CRIT << "Ignoring call to launchWithScript. getpwuid(…) failed.";

        else if (QFile file(QDir(cacheLocation()).filePath("terminal_command"));
                 !file.open(QIODevice::WriteOnly))
            WARN << QString("Running command in %1 failed. Could not create temporary file: %2")
                    .arg(name(), file.errorString());
        else
        {
            // Note for future self
            // QTemporaryFile does not start
            // Deleting the file introduces race condition

            if (!working_dir.isEmpty())
                file.write(QString(R"(cd "%1";)").arg(working_dir).toUtf8());
            file.write("clear;");
            file.write(script.toUtf8());
            file.close();

            auto command = QString("%1 -i %2").arg(pwd->pw_shell, file.fileName());

            albert::runDetachedProcess({"/usr/bin/osascript", "-l", "AppleScript",
                                        "-e", QString(apple_script_).arg(command)});
        }
    }
};






Plugin::Plugin()
{
    ::plugin = this;

    auto s = settings();
    commonInitialize(s);

    indexer.parallel = [this](const bool &abort)
    {
        vector<shared_ptr<applications::Application>> apps;

        apps.emplace_back(make_shared<Application>("/System/Library/CoreServices/Finder.app",
                                                   use_non_localized_name_));

        for (const auto &path : appDirectories())
            for (const auto &fi : QDir(path).entryInfoList({"*.app"}))
                if (abort)
                    return apps;
                else
                    try {
                        apps.emplace_back(make_shared<Application>(fi.absoluteFilePath(),
                                                                   use_non_localized_name_));
                    } catch (const runtime_error &e) {
                        WARN << e.what();
                    }

        return apps;
    };

    indexer.finish = [this](auto &&result)
    {
        INFO << QString("Indexed %1 applications.").arg(result.size());
        applications = ::move(result);

        ranges::sort(applications, [](const auto &a, const auto &b){ return a->id() < b->id(); });

        for (const auto &app : applications)
            INFO << QString("%1: %2").arg(app->id(), static_pointer_cast<Application>(app)->names().join(", "));

        // Add terminals

        terminals.clear();

        auto id = QStringLiteral("com.apple.Terminal");
        if (auto it = ranges::find_if(applications,
                                      [&](const auto &app){ return app->id() == id; });
                it != applications.end())
            terminals.emplace_back(make_unique<AppleScriptLaunchableTerminal>(
                *it,
                R"(tell application "Terminal" to activate
                   tell application "Terminal" to do script "exec %1")"
                ));

        id = QStringLiteral("com.googlecode.iterm2");
        if (auto it = ranges::find_if(applications,
                                      [&](const auto &app){ return app->id() == id; });
                it != applications.end())
            terminals.emplace_back(make_unique<AppleScriptLaunchableTerminal>(
                *it,
                R"(tell application "iTerm" to create window with default profile command "%1")"
            ));

        setUserTerminalFromConfig();

        // Build index items

        vector<IndexItem> ii;
        for (const auto &app : applications)
            for (const auto &name : static_pointer_cast<Application>(app)->names())
                ii.emplace_back(static_pointer_cast<Application>(app), name);
        setIndexItems(::move(ii));
    };

}

Plugin::~Plugin() = default;

shared_ptr<applications::Terminal> Plugin::userTerminal() const
{
    return user_terminal;
}

void Plugin::runTerminal(const QString &script, const QString &working_dir) const
{
    if (!user_terminal)
        QMessageBox::warning(nullptr, {},
                             tr("Failed to run terminal. No supported terminal available."));
    else if (script.isEmpty())
        user_terminal->launch(working_dir);
    else
        user_terminal->launchWithScript(script, working_dir);
}

void Plugin::updateIndexItems() { indexer.run(); }

QWidget *Plugin::buildConfigWidget()
{
    auto *w = new QWidget;
    Ui::ConfigWidget ui;
    ui.setupUi(w);

    ALBERT_PROPERTY_CONNECT_CHECKBOX(this, use_non_localized_name, ui.checkBox_useNonLocalizedName);

    ui.formLayout->addRow(tr("Terminal"), createTerminalFormWidget());

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

