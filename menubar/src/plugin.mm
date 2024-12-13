// Copyright (c) 2024-2024 Manuel Schneider

#include "plugin.h"
#include <Cocoa/Cocoa.h>
#include <QKeySequence>
#include <QMessageBox>
#include <QUrl>
#include <albert/logging.h>
#include <albert/matcher.h>
using namespace albert;
using namespace std;
ALBERT_LOGGING_CATEGORY("menu")
#if  ! __has_feature(objc_arc)
#error This file must be compiled with ARC.
#endif

static Qt::KeyboardModifiers toQt(AXMenuItemModifiers command_modifiers)
{
    Qt::KeyboardModifiers qt_modifiers;

    if (command_modifiers & kAXMenuItemModifierShift)
        qt_modifiers.setFlag(Qt::ShiftModifier);

    if (command_modifiers & kAXMenuItemModifierOption)
        qt_modifiers.setFlag(Qt::AltModifier);

    if (command_modifiers & kAXMenuItemModifierControl)
        qt_modifiers.setFlag(Qt::MetaModifier);

    if (!(command_modifiers & kAXMenuItemModifierNoCommand))
        qt_modifiers.setFlag(Qt::ControlModifier); // see AXMenuItemModifiers

    return qt_modifiers;
}

// See
// https://github.com/216k155/MacOSX-SDKs/blob/master/MacOSX10.11.sdk/System/Library/Frameworks/
//   Carbon.framework/Versions/A/Frameworks/HIToolbox.framework/Versions/A/Headers/Menus.h
// https://github.com/Hammerspoon/hammerspoon/blob/master/extensions/application/application.lua
static const map<char, const char*> glyph_map
{
    // {0x00, ""},  // Null (always glyph 1)
    {0x02, "⇥"},  // Tab to the right key (for left-to-right script systems)
    {0x03, "⇤"},  // Tab to the left key (for right-to-left script systems)
    {0x04, "⌤"},  // Enter key
    {0x05, "⇧"},  // Shift key
    {0x06, "⌃"},  // Control key
    {0x07, "⌥"},  // Option key
    {0x09, "␣"},  // Space (always glyph 3) key
    {0x0A, "⌦"},  // Delete to the right key (for right-to-left script systems)
    {0x0B, "↩"},  // Return key (for left-to-right script systems)
    {0x0C, "↪"},  // Return key (for right-to-left script systems)
    {0x0D, "↩"},  // Nonmarking return key
    {0x0F, ""},  // Pencil key
    {0x10, "⇣"},  // Downward dashed arrow key
    {0x11, "⌘"},  // Command key
    {0x12, "✓"},  // Checkmark key
    {0x13, "◇"},  // Diamond key
    {0x14, ""},  // Apple logo key (filled)
    // {0x15, ""},  // Unassigned (paragraph in Korean)
    {0x17, "⌫"},  // Delete to the left key (for left-to-right script systems)
    {0x18, "⇠"},  // Leftward dashed arrow key
    {0x19, "⇡"},  // Upward dashed arrow key
    {0x1A, "⇢"},  // Rightward dashed arrow key
    {0x1B, "⎋"},  // Escape key
    {0x1C, "⌧"},  // Clear key
    {0x1D, "『"},  // Unassigned (left double quotes in Japanese)
    {0x1E, "』"},  // Unassigned (right double quotes in Japanese)
    // {0x1F, ""},  // Unassigned (trademark in Japanese)
    {0x61, "␢"},  // Blank key
    {0x62, "⇞"},  // Page up key
    {0x63, "⇪"},  // Caps lock key
    {0x64, "←"},  // Left arrow key
    {0x65, "→"},  // Right arrow key
    {0x66, "↖"},  // Northwest arrow key
    {0x67, "﹖"},  // Help key
    {0x68, "↑"},  // Up arrow key
    {0x69, "↘"},  // Southeast arrow key
    {0x6A, "↓"},  // Down arrow key
    {0x6B, "⇟"},  // Page down key
    {0x6C, ""},  // Apple logo key (outline)
    {0x6D, ""},  // Contextual menu key
    {0x6E, "⌽"},  // Power key
    {0x6F, "F1"},  // F1 key
    {0x70, "F2"},  // F2 key
    {0x71, "F3"},  // F3 key
    {0x72, "F4"},  // F4 key
    {0x73, "F5"},  // F5 key
    {0x74, "F6"},  // F6 key
    {0x75, "F7"},  // F7 key
    {0x76, "F8"},  // F8 key
    {0x77, "F9"},  // F9 key
    {0x78, "F10"},  // F10 key
    {0x79, "F11"},  // F11 key
    {0x7A, "F12"},  // F12 key
    {0x87, "F13"},  // F13 key
    {0x88, "F14"},  // F14 key
    {0x89, "F15"},  // F15 key
    {0x8A, "⎈"},  // Control key (ISO standard)
    {0x8C, "⏏"},  // Eject key (available on Mac OS X 10.2 and later)
    {0x8D, "英数"},  // Japanese eisu key (available in Mac OS X 10.4 and later)
    {0x8E, "かな"},  // Japanese kana key (available in Mac OS X 10.4 and later)
    {0x8F, "F16"},  // F16 key (available in SnowLeopard and later)
    {0x90, "F17"},  // F17 key (available in SnowLeopard and later)
    {0x91, "F18"},  // F18 key (available in SnowLeopard and later)
    {0x92, "F19"}   // F19 key (available in SnowLeopard and later)
};

struct MenuItem : public albert::Item
{
    MenuItem(AXUIElementRef e, QStringList p, const QString &s, const QString &i)
        : element(e), path(p), shortcut(s), icon_url(i) { CFRetain(element); }

    ~MenuItem() { CFRelease(element); };

    QString id() const override { return path.join(QString()); }

    QString text() const override { return path.last(); }

    QString subtext() const override {
        return shortcut.isEmpty() ?
                   pathString() : QStringLiteral("%1 (%2)").arg(pathString(), shortcut);
    }
    QStringList iconUrls() const override { return {icon_url}; }

    QString inputActionText() const override { return text(); }

    std::vector<albert::Action> actions() const override
    {
        return {{
            "activate", Plugin::tr("Activate"),
            [this] {
                if (auto err = AXUIElementPerformAction(element, kAXPressAction);
                    err != kAXErrorSuccess)
                    WARN << "Failed to activate menu item";
            }
        }};
    }

    QString pathString() const { return path.join(" → "); }

    AXUIElementRef element;
    const QStringList path;
    const QString shortcut;
    const QString icon_url;
};

Qt::KeyboardModifiers convertCFModifiersToQtModifiers(int cfModifiers)
{
    Qt::KeyboardModifiers qtModifiers = Qt::NoModifier;
    if (cfModifiers & kCGEventFlagMaskShift)       qtModifiers |= Qt::ShiftModifier;
    if (cfModifiers & kCGEventFlagMaskControl)     qtModifiers |= Qt::ControlModifier;
    if (cfModifiers & kCGEventFlagMaskAlternate)   qtModifiers |= Qt::AltModifier;
    if (cfModifiers & kCGEventFlagMaskCommand)     qtModifiers |= Qt::MetaModifier;
    if (cfModifiers & kCGEventFlagMaskSecondaryFn) qtModifiers |= Qt::GroupSwitchModifier;
    return qtModifiers;
}

static void retrieveMenuItemsRecurse(const bool & valid,
                                     vector<shared_ptr<MenuItem>>& items,
                                     const QString &icon_url,
                                     QStringList path,
                                     AXUIElementRef element)
{
    if (!valid)
        return;

    // Define attribute names to fetch at once
    enum AXKeys {
        Enabled,
        Title,
        Children,
        MenuItemCmdChar,
        MenuItemCmdGlyph,
        MenuItemCmdModifiers,
    };
    CFStringRef ax_attributes[] = {
        kAXEnabledAttribute,
        kAXTitleAttribute,
        kAXChildrenAttribute,
        kAXMenuItemCmdCharAttribute,
        kAXMenuItemCmdGlyphAttribute,
        kAXMenuItemCmdModifiersAttribute,
    };
    CFArrayRef attributes_array = CFArrayCreate(nullptr,
                                               (const void **) ax_attributes,
                                               sizeof(ax_attributes) / sizeof(ax_attributes[0]),
                                               &kCFTypeArrayCallBacks);
    CFArrayRef attribute_values = nullptr;
    auto error = AXUIElementCopyMultipleAttributeValues(element,
                                                        attributes_array,
                                                        0,
                                                        &attribute_values);

    if (error != kAXErrorSuccess)
        WARN << QString("Failed to retrieve multiple attributes: %1 (See AXError.h)").arg(error);
    else if(!attribute_values)
        WARN << QString("Failed to retrieve multiple attributes: Returned null.");
    else if(CFGetTypeID(attribute_values) != CFArrayGetTypeID())
        WARN << QString("Failed to retrieve multiple attributes: Returned type is not array.");
    else{

        class skip : exception {};

        try {

            // Get enabled state (kAXEnabledAttribute)

            auto value = CFArrayGetValueAtIndex(attribute_values, AXKeys::Enabled);
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
                value = CFArrayGetValueAtIndex(attribute_values, AXKeys::Title);
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

            value = CFArrayGetValueAtIndex(attribute_values, AXKeys::Children);
            if (!value)
                throw runtime_error("Fetched kAXChildrenAttribute is null");
            else if (CFGetTypeID(value) == kAXValueAXErrorType)
                throw runtime_error("Fetched kAXChildrenAttribute is kAXValueAXErrorType");
            else if (CFGetTypeID(value) != CFArrayGetTypeID())
                throw runtime_error("Fetched kAXChildrenAttribute is not of type CFArrayRef");
            else if (CFArrayGetCount((CFArrayRef)value) > 0)
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

                    retrieveMenuItemsRecurse(valid, items, icon_url, path, (AXUIElementRef)value);
                }
            }
            else if (CFArrayRef actions = nullptr;
                     AXUIElementCopyActionNames(element, &actions) == kAXErrorSuccess && actions)
            {

                QString command_char;
                if (auto v = CFArrayGetValueAtIndex(attribute_values, AXKeys::MenuItemCmdChar);
                    v && CFGetTypeID(v) == CFStringGetTypeID())
                    command_char = QString::fromCFString((CFStringRef)v);

                // if there is a glyph use that instead
                if (auto v = CFArrayGetValueAtIndex(attribute_values, AXKeys::MenuItemCmdGlyph);
                    v && CFGetTypeID(v) == CFNumberGetTypeID())
                {
                    int glyphID = [(__bridge NSNumber*)v intValue];
                    if (auto it = glyph_map.find(glyphID); it != glyph_map.end())
                        command_char = it->second;
                }

                Qt::KeyboardModifiers mods;
                if (auto v = CFArrayGetValueAtIndex(attribute_values, AXKeys::MenuItemCmdModifiers);
                    v && CFGetTypeID(v) == CFNumberGetTypeID())
                    mods = toQt([(__bridge NSNumber*)v intValue]);

                QString shortcut;
                if (!command_char.isEmpty())
                    shortcut = QKeySequence(mods).toString(QKeySequence::NativeText) + command_char;

                if (CFArrayContainsValue(actions,
                                         CFRangeMake(0, CFArrayGetCount(actions)),
                                         kAXPressAction))
                    items.emplace_back(make_shared<MenuItem>(element, path, shortcut, icon_url));

                CFRelease(actions);
            }
        } catch (const skip &e) {
        } catch (const exception &e) {
            DEBG << e.what();
        }

        CFRelease(attribute_values);
    }
    CFRelease(attributes_array);
}

static vector<shared_ptr<MenuItem>> retrieveMenuBarItems(const bool &valid)
{
    vector<shared_ptr<MenuItem>> menu_items;
    auto app = NSWorkspace.sharedWorkspace.frontmostApplication;
    auto app_icon_url = QString("qfip:" + QUrl::fromNSURL(app.bundleURL).toLocalFile());
    auto app_ax = AXUIElementCreateApplication(app.processIdentifier);

    CFTypeRef app_ax_menu_bar = nullptr;
    if (auto error = AXUIElementCopyAttributeValue(app_ax, kAXMenuBarAttribute, &app_ax_menu_bar);
        error != kAXErrorSuccess)
        WARN << QString("Failed to retrieve menubar: %1 (See AXError.h)").arg(error);
    else if (!app_ax_menu_bar)
        WARN << QString("Failed to retrieve menubar: Returned null.");
    else {
        CFTypeRef ax_menus = nullptr;
        if (error = AXUIElementCopyAttributeValue((AXUIElementRef)app_ax_menu_bar,
                                                  kAXChildrenAttribute,
                                                  &ax_menus);
            error != kAXErrorSuccess)
            WARN << QString("Failed to retrieve menu bar menus: %1 (See AXError.h)").arg(error);
        else if (!ax_menus)
            WARN << QString("Failed to retrieve menu bar menus: Returned null.");
        else {
            // Skip "Apple" menu
            for (CFIndex i = 1, c = CFArrayGetCount((CFArrayRef) ax_menus); i < c; ++i)
                retrieveMenuItemsRecurse(
                    valid, menu_items, app_icon_url, {},
                    (AXUIElementRef)CFArrayGetValueAtIndex((CFArrayRef) ax_menus, i)
                );

            CFRelease(ax_menus);
        }
        CFRelease(app_ax_menu_bar);
    }
    CFRelease(app_ax);

    return menu_items;
}

class Plugin::Private
{
public:
    bool fuzzy;
    std::vector<std::shared_ptr<MenuItem>> menu_items;
    pid_t current_menu_pid = 0;
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

        // Note: does not add an entry to the privacy settings in debug mode
        NSString* prefPage = @"x-apple.systempreferences:com.apple.preference.security?Privacy_Accessibility";
        [[NSWorkspace sharedWorkspace] openURL:(NSURL * __nonnull)[NSURL URLWithString:prefPage]];
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

    // Update menu if app changed
    auto app = NSWorkspace.sharedWorkspace.frontmostApplication;
    if (app && d->current_menu_pid != app.processIdentifier)
    {
        d->current_menu_pid = app.processIdentifier;

        // AX api is not thread save, dispatch in main thread
        __block vector<shared_ptr<MenuItem>> menu_items;
        dispatch_semaphore_t semaphore = dispatch_semaphore_create(0);
        dispatch_async(dispatch_get_main_queue(), ^{
            menu_items = retrieveMenuBarItems(query->isValid());
            dispatch_semaphore_signal(semaphore);
        });
        dispatch_semaphore_wait(semaphore, DISPATCH_TIME_FOREVER);  // Wait for user

        d->menu_items = ::move(menu_items);
    }

    vector<RankItem> results;
    for (const auto& item : d->menu_items)
        if (auto m = Matcher(query->string(), {.fuzzy = d->fuzzy})
                         .match(item->text(), item->pathString()); m)
            results.emplace_back(item, m);

    return results;
}
