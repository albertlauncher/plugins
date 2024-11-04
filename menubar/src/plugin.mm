// Copyright (c) 2024-2024 Manuel Schneider

#include "plugin.h"
#include <QUrl>
#include <Cocoa/Cocoa.h>
#include <albert/logging.h>
#include <albert/matcher.h>
#include <QMessageBox>
using namespace albert;
using namespace std;
ALBERT_LOGGING_CATEGORY("menu")


/// Volatile lazy menu item.
class MenuItem : public albert::Item
{
public:
    MenuItem(QStringList path, AXUIElementRef element, NSRunningApplication *app):
        path_(path),
        element_(element),
        app_(app)
    {
        // INFO << buildPathRecurse(element_).join(" > ");
        // CRIT << "Menu" << QString::fromNSString(title);
        CFRetain(element);
    }

    ~MenuItem() { CFRelease(element_); };

    QString id() const override { return path_.join(" > "); }
    QString text() const override { return path_.last(); }
    QString subtext() const override { return path_.join(" > "); }
    QStringList iconUrls() const override {
        return icon_urls_.isEmpty()
                   ? icon_urls_ = QStringList("qfip:" + QUrl::fromNSURL(app_.bundleURL).toLocalFile())
                   : icon_urls_;
    }
    QString inputActionText() const override { return path_.join(" "); }
    std::vector<albert::Action> actions() const override
    {
        return {{
            "activate", Plugin::tr("Activate"),
            [this] {
                if (auto err = AXUIElementPerformAction(element_, kAXPressAction);
                    err != kAXErrorSuccess)
                    WARN << "Failed to activate menu item";
            }
        }};
    }

private:

    QStringList path_;
    AXUIElementRef element_;
    NSRunningApplication *app_;
    mutable QStringList icon_urls_;  // Fetched lazy

};

static void retrieveMenuItemsRecurse(const bool & valid,
                                     vector<shared_ptr<Item>>& items,
                                     NSRunningApplication *app,
                                     QStringList path,
                                     AXUIElementRef element,
                                     int depth = 0)
{
    if (!valid)
        return;

    // Define attribute names to fetch at once
    CFStringRef attributes[] = {kAXEnabledAttribute,
                                kAXTitleAttribute,
                                kAXChildrenAttribute,
                                kAXRoleAttribute,
                                kAXSubroleAttribute};
    CFArrayRef attributes_array = CFArrayCreate(nullptr,
                                               (const void **) attributes,
                                               sizeof(attributes) / sizeof(attributes[0]),
                                               &kCFTypeArrayCallBacks);
    CFArrayRef attribute_values = nullptr;
    auto error = AXUIElementCopyMultipleAttributeValues(element, attributes_array,
                                                        0, &attribute_values);

    if (error != kAXErrorSuccess)
        WARN << QString("Failed to retrieve multiple attributes: %1 (See AXError.h)").arg(error);
    else if(!attribute_values)
        WARN << QString("Failed to retrieve multiple attributes: Returned null.");
    else if(CFGetTypeID(attribute_values) != CFArrayGetTypeID())
        WARN << QString("Failed to retrieve multiple attributes: Returned type is not array.");
    else{

        class skip : exception
        {};

        try {

            // Get enabled state (kAXEnabledAttribute)

            auto value = CFArrayGetValueAtIndex(attribute_values, 0);
            if (!value)
                throw runtime_error("Fetched kAXEnabledAttribute is null");
            else if (CFGetTypeID(value) == kAXValueAXErrorType)
                throw runtime_error("Fetched kAXEnabledAttribute is kAXValueAXErrorType");
            else if (CFGetTypeID(value) != CFBooleanGetTypeID())
                throw runtime_error("Fetched kAXEnabledAttribute is not of type CFBooleanRef");
            else if (!CFBooleanGetValue((CFBooleanRef)value))  // Skip disabled ones
                // throw runtime_error("AXUIElement is disabled (kAXEnabledAttribute)");
                throw skip();


            // Get title (kAXTitleAttribute), skip empty titles
            // Title is optional for recursion but mandatory for items

            QString title;

            try {
                value = CFArrayGetValueAtIndex(attribute_values, 1);
                if (!value)
                    throw runtime_error("Fetched kAXTitleAttribute is null");

                else if (CFGetTypeID(value) == kAXValueAXErrorType)
                    throw runtime_error("Fetched kAXTitleAttribute is kAXValueAXErrorType");

                // expected, menus
                else if (CFGetTypeID(value) == CFStringGetTypeID()
                         && CFStringGetLength((CFStringRef)value) == 0)
                    throw runtime_error("AXUIElement title is empty");

                // expected, menus, maybe coordinates or sth
                // else if (CFGetTypeID(value) == AXValueGetTypeID());

                else if (CFGetTypeID(value) == CFStringGetTypeID())
                {
                    title = QString::fromCFString((CFStringRef)value).trimmed();
                    path << title;
                }

                // expected, menus have no title
                // else if (CFGetTypeID(value) != CFStringGetTypeID())


            } catch (const exception &e) {
                // path << "N/A";
                WARN << e.what();
            }


            // Get children (kAXChildrenAttribute)

            value = CFArrayGetValueAtIndex(attribute_values, 2);
            if (!value)
                throw runtime_error("Fetched kAXChildrenAttribute is null");
            else if (CFGetTypeID(value) == kAXValueAXErrorType)
                throw runtime_error("Fetched kAXChildrenAttribute is kAXValueAXErrorType");
            else if (CFGetTypeID(value) != CFArrayGetTypeID())
                throw runtime_error("Fetched kAXChildrenAttribute is not of type CFArrayRef");
            else if (CFArrayGetCount((CFArrayRef)value) == 0)
            {
                if (CFArrayRef actions = nullptr;
                    AXUIElementCopyActionNames(element, &actions) == kAXErrorSuccess && actions)
                {
                    if (CFArrayContainsValue(actions,
                                             CFRangeMake(0, CFArrayGetCount(actions)),
                                             kAXPressAction))
                    {
                        // DEBG << path.join(">");
                        items.emplace_back(make_shared<MenuItem>(path, element, app));
                    }

                    // for (CFIndex i = 0, c = CFArrayGetCount(actions); i < c; ++i)
                    //     DEBG << QString::fromCFString((CFStringRef)CFArrayGetValueAtIndex(actions, i));

                    CFRelease(actions);
                }
            }
            else
            {
                // Recursively process children

                auto cf_children = (CFArrayRef)value;
                for (CFIndex i = 0, c = CFArrayGetCount(cf_children); i < c; ++i)
                {
                    value = CFArrayGetValueAtIndex(cf_children, i);
                    if (!value)
                        throw runtime_error("Fetched child is null");
                    else if (CFGetTypeID(value) == kAXValueAXErrorType)
                        throw runtime_error("Fetched child is kAXValueAXErrorType");
                    else if (CFGetTypeID(value) != AXUIElementGetTypeID())
                        throw runtime_error("Fetched child is not of type AXUIElementRef");

                    retrieveMenuItemsRecurse(valid, items, app, path, (AXUIElementRef)value, depth +1 );
                }
            }
        } catch (const skip &e) {
        } catch (const exception &e) {
            DEBG << depth << e.what();
        }

        CFRelease(attribute_values);
    }
    CFRelease(attributes_array);
}

static vector<shared_ptr<Item>> retrieveMenuBarItems(const bool &valid, NSRunningApplication *app)
{
    vector<shared_ptr<Item>> menu_items;

    auto ax_app = AXUIElementCreateApplication(app.processIdentifier);

    CFTypeRef ax_menu_bar = nullptr;
    if (auto error = AXUIElementCopyAttributeValue(ax_app, kAXMenuBarAttribute, &ax_menu_bar);
        error != kAXErrorSuccess)
        WARN << QString("Failed to retrieve menubar: %1 (See AXError.h)").arg(error);
    else if (!ax_menu_bar)
        WARN << QString("Failed to retrieve menubar: Returned null.");
    else {
        CFTypeRef ax_menus = nullptr;
        if (error = AXUIElementCopyAttributeValue((AXUIElementRef) ax_menu_bar,
                                                  kAXChildrenAttribute,
                                                  &ax_menus);
            error != kAXErrorSuccess)
            WARN << QString("Failed to retrieve menu bar menus: %1 (See AXError.h)").arg(error);
        else if (!ax_menus)
            WARN << QString("Failed to retrieve menu bar menus: Returned null.");
        else {
            // Skip "Apple" menu
            for (CFIndex i = 1, c = CFArrayGetCount((CFArrayRef) ax_menus); i < c; ++i)
                retrieveMenuItemsRecurse(valid,
                                         menu_items,
                                         app,
                                         {},
                                         (AXUIElementRef)
                                             CFArrayGetValueAtIndex((CFArrayRef) ax_menus, i));

            CFRelease(ax_menus);
        }
        CFRelease(ax_menu_bar);
    }
    CFRelease(ax_app);

    return menu_items;
}

class Plugin::Private
{
public:
    bool fuzzy;
    std::vector<std::shared_ptr<albert::Item>> menu_items;
    NSRunningApplication *app;
};

Plugin::Plugin() : d(make_unique<Private>())
{
    if (!AXIsProcessTrusted())
    {
        DEBG << "Accessibility permission denied.";
        QMessageBox::information(nullptr, "",
                                 tr("The menu bar plugin requires accessibility permissions to "
                                    "access the menu items of the focused application.\n\n"
                                    "macOS requires you to enable this manually in system "
                                    "settings. Please toggle Albert in the accessibility settings, "
                                    "which will appear after you close this dialog."));

        NSString* prefPage = @"x-apple.systempreferences:com.apple.preference.security?Privacy_Accessibility";
        [[NSWorkspace sharedWorkspace] openURL:[NSURL URLWithString:prefPage]];

        // NSDictionary *options = @{(__bridge NSString *)kAXTrustedCheckOptionPrompt: @YES};
        // BOOL isTrusted = AXIsProcessTrustedWithOptions((__bridge CFDictionaryRef)options);
    }
}

Plugin::~Plugin() = default;

QString Plugin::defaultTrigger() const { return "m "; }

bool Plugin::supportsFuzzyMatching() const { return true; }

void Plugin::setFuzzyMatching(bool enabled) { d->fuzzy = enabled; }

vector<RankItem> Plugin::handleGlobalQuery(const Query *query)
{
    if (!AXIsProcessTrusted())
    {
        WARN << "Accessibility permission denied.";
        return {};
    }

    auto app = NSWorkspace.sharedWorkspace.frontmostApplication;

    // Update menu if app changed
    if (app.bundleIdentifier != d->app.bundleIdentifier)
    {
        d->app = app;

        // AX api is not thread save, dispatch in main thread
        __block vector<shared_ptr<Item>> menu_items;
        dispatch_semaphore_t semaphore = dispatch_semaphore_create(0);
        dispatch_async(dispatch_get_main_queue(), ^{
            menu_items = retrieveMenuBarItems(query->isValid(), app);
            dispatch_semaphore_signal(semaphore);
        });
        dispatch_semaphore_wait(semaphore, DISPATCH_TIME_FOREVER);  // Wait for user

        d->menu_items = ::move(menu_items);
    }

    vector<RankItem> results;
    Matcher matcher(query->string(), {.fuzzy = d->fuzzy});

    for (const auto& item : d->menu_items)
    {
        // Prefer item matches over path matches
        if (auto m = matcher.match(item->text()); m)
            results.emplace_back(item, m);
        else if (m = matcher.match(item->subtext()); m)
            results.emplace_back(item, m);
    }

    return results;
}
