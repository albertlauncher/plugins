// Copyright (c) 2023-2024 Manuel Schneider

#include "plugin.h"
#include <Contacts/Contacts.h>
#include <albert/backgroundexecutor.h>
#include <albert/item.h>
#include <albert/logging.h>
#include <albert/util.h>
ALBERT_LOGGING_CATEGORY("contacts")
using namespace albert;
using namespace std;


class Plugin::Private
{
public:
    CNContactStore *store = [[CNContactStore alloc] init];
    BackgroundExecutor<vector<IndexItem>> indexer;
    NSObject *observer;
    QString tr_person = Plugin::tr("Person");
    QString tr_organization = Plugin::tr("Organization");
};


class ContactItem : public Item
{
    // Item interface
public:
    ContactItem(const QString id, const QString name, CNContactType contactType);

    QString id() const override;
    QString text() const override;
    QString subtext() const override;
    QStringList iconUrls() const override;
    QString inputActionText() const override;
    vector<Action> actions() const override;

    QString id_;
    QString name_;
    CNContactType type_;
    __weak static CNContactStore *store_;
};

__weak CNContactStore *ContactItem::store_ = nil;

ContactItem::ContactItem(const QString id, const QString name, CNContactType type):
    id_(id), name_(name), type_(type)
{}

QString ContactItem::id() const { return id_; }

QString ContactItem::text() const { return name_; }

QString ContactItem::subtext() const
{
    switch (type_) {
    case CNContactTypePerson:
        return Plugin::tr("Person");
    case CNContactTypeOrganization:
        return Plugin::tr("Organization");
    }
}

QStringList ContactItem::iconUrls() const
{
    return {QStringLiteral("qfip:/System/Applications/Contacts.app")};
}

QString ContactItem::inputActionText() const { return name_; }

vector<Action> ContactItem::actions() const
{
    vector<Action> actions;

    actions.emplace_back("cn-open", Plugin::tr("Open in contacts app"), [this] {
        // QUrl refuses to parse addressboolk urls. Use platform open.
        // https://bugreports.qt.io/browse/QTBUG-129496
        albert::runDetachedProcess({"open", "addressbook://" + id_});
    });

    NSError *error = nil;
    auto *contact = [store_ unifiedContactWithIdentifier:id_.toNSString()
                                             keysToFetch:@[CNContactPhoneNumbersKey,
                                                           CNContactEmailAddressesKey,
                                                           CNContactUrlAddressesKey]
                                                   error:&error];

    if (error) {
        WARN << "Error fetching contacts:" << QString::fromNSString([error localizedDescription]);
        return actions;
    }

    // Phone numbers
    for (CNLabeledValue<CNPhoneNumber *> *number in contact.phoneNumbers)
    {
        auto val = QString::fromNSString(number.value.stringValue);
        QString label;
        if (number.label == nil || number.label.length == 0)
            label = Plugin::tr("Phone");
        else if (auto *l = [CNLabeledValue localizedStringForLabel:number.label]; l)
            label = QString::fromNSString(l);
        else
            label = QString::fromNSString(number.label);

        actions.emplace_back("pn-copy", Plugin::tr("Copy phone number '%1'").arg(label),
                             [=]{ setClipboardText(val); });
        actions.emplace_back("pn-call", Plugin::tr("Call phone number '%1'").arg(label),
                             [=]{ openUrl("tel:" + val); });
        actions.emplace_back("pn-iMess", Plugin::tr("iMessage to '%1'").arg(label),
                             [=]{ openUrl("sms:" + val); });
    }

    // Email addresses
    for (CNLabeledValue<NSString *> *mail in contact.emailAddresses)
    {
        auto val = QString::fromNSString(mail.value);
        QString label;
        if (mail.label == nil || mail.label.length == 0)
            label = Plugin::tr("Email");
        else if (auto *l = [CNLabeledValue localizedStringForLabel:mail.label]; l)
            label = QString::fromNSString(l);
        else
            label = QString::fromNSString(mail.label);

        actions.emplace_back("ea-copy", Plugin::tr("Copy email address '%1'").arg(label),
                             [=]{ setClipboardText(val); });
        actions.emplace_back("ea-mail", Plugin::tr("Send mail to '%1'").arg(label),
                             [=]{ openUrl("mailto:" + val); });
    }

    // Url addresses
    for (CNLabeledValue<NSString *> *url in contact.urlAddresses)
    {
        auto val = QString::fromNSString(url.value);
        QString label;
        if (url.label == nil || url.label.length == 0)
            label = Plugin::tr("Website");
        else if (auto *l = [CNLabeledValue localizedStringForLabel:url.label]; l)
            label = QString::fromNSString(l);
        else
            label = QString::fromNSString(url.label);


        actions.emplace_back("ua-copy", Plugin::tr("Copy website address '%1'").arg(label),
                             [=]{ setClipboardText(val); });
        actions.emplace_back("ua-open", Plugin::tr("Open website '%1'").arg(label),
                             [=]{ openUrl(val); });
    }

    return actions;
}

Plugin::Plugin() : d(make_unique<Private>())
{
    switch ([CNContactStore authorizationStatusForEntityType:CNEntityTypeContacts])
    {
    case CNAuthorizationStatusRestricted: {
        const char *m = QT_TR_NOOP("The application is not authorized to access contacts.");
        WARN << m;
        throw runtime_error(tr(m).toStdString());
    }
    case CNAuthorizationStatusDenied: {
        const char *m = QT_TR_NOOP("The user denied access to contacts.");
        WARN << m;
        throw runtime_error(tr(m).toStdString());
    }
    case CNAuthorizationStatusNotDetermined: {
        INFO << "Requesting access to contact data.";
        __block bool granted = false;
        __block NSError * error;

        dispatch_semaphore_t semaphore = dispatch_semaphore_create(0);

        [d->store requestAccessForEntityType:CNEntityTypeContacts
                           completionHandler:^(BOOL grant, NSError *_Nullable err) {
                               error = err;
                               granted = grant;
                               dispatch_semaphore_signal(semaphore);
                           }];

        dispatch_semaphore_wait(semaphore, DISPATCH_TIME_FOREVER);  // Wait for user

        if (error)
            throw runtime_error([error.description UTF8String]);

        if (!granted)
            throw runtime_error(tr("The user denied access to contacts.").toStdString());

        INFO << "User granted access to contacts";
    }
    default:
        // CNAuthorizationStatusAuthorized: all good
        // CNAuthorizationStatusLimited: not available on macos
        break;
    }

    d->store = [[CNContactStore alloc] init];
    ContactItem::store_ = d->store;

    d->indexer.parallel = [this](const bool &) -> vector<IndexItem>
    {
        INFO << "Indexing contacts";

        __block vector<IndexItem> index_items;

        auto *keys = @[
            // CNContactIdentifierKey, always fetched
            CNContactTypeKey,
            CNContactNamePrefixKey,
            CNContactGivenNameKey,
            CNContactMiddleNameKey,
            CNContactFamilyNameKey,
            CNContactNameSuffixKey,
            CNContactOrganizationNameKey,
            CNContactDepartmentNameKey,
        ];

        auto block = ^(CNContact *contact, BOOL *) {
            @autoreleasepool {
                auto name = QStringList({
                    QString::fromNSString(contact.organizationName),
                    QString::fromNSString(contact.departmentName),
                    QString::fromNSString(contact.namePrefix),
                    QString::fromNSString(contact.givenName),
                    QString::fromNSString(contact.middleName),
                    QString::fromNSString(contact.familyName),
                    QString::fromNSString(contact.nameSuffix),
                }).join(" ").simplified();

                if (name.isEmpty())
                    DEBG << "Empty name. Skipping contact" << QString::fromNSString(contact.identifier);
                else
                {
                    auto item = make_shared<ContactItem>(
                        QString::fromNSString(contact.identifier),
                        name,
                        contact.contactType);

                    index_items.emplace_back(::move(item), name);
                }
            }
        };

        auto *request = [[CNContactFetchRequest alloc] initWithKeysToFetch:keys];
        NSError *error = nil;
        [d->store enumerateContactsWithFetchRequest:request
                                              error:&error
                                         usingBlock:block];
        if (error)
            WARN << "Error fetching contacts:" << QString::fromNSString([error localizedDescription]);

        return index_items;
    };

    d->indexer.finish = [this](vector<IndexItem> &&r)
    {
        INFO << QString("Indexed %1 contact items (%2 ms).")
                    .arg(r.size()).arg(d->indexer.runtime.count());

        setIndexItems(::move(r));
    };

    // Register for contact changes
    d->observer = [[NSNotificationCenter defaultCenter]
        addObserverForName:CNContactStoreDidChangeNotification
                    object:nil
                     queue:[NSOperationQueue mainQueue]
                usingBlock:^(NSNotification *_Nonnull) { d->indexer.run(); }];
}

Plugin::~Plugin()
{
    [[NSNotificationCenter defaultCenter] removeObserver:d->observer];
}

void Plugin::updateIndexItems() { d->indexer.run(); }
