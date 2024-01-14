// Copyright (c) 2023 Manuel Schneider

#include "albert/albert.h"
#include "albert/extension/queryhandler/standarditem.h"
#include "albert/logging.h"
#include "plugin.h"
#include <Contacts/Contacts.h>
#include <QLabel>
#include <QRegularExpression>
ALBERT_LOGGING_CATEGORY("contacts")
using namespace albert;
using namespace std;


vector<RankItem> Plugin::handleGlobalQuery(const GlobalQuery *query) const
{
  vector<RankItem> results;
  @autoreleasepool {
    CNContactStore *store = [[CNContactStore alloc] init];


    CNAuthorizationStatus authorizationStatus
      = [CNContactStore authorizationStatusForEntityType:CNEntityTypeContacts];
    switch (authorizationStatus) {
      case CNAuthorizationStatusRestricted: {
        WARN << "Access to contacts restricted.";
        return results;
      }
      case CNAuthorizationStatusDenied:{
        WARN << "Access to contacts denied.";
        return results;
      }
      case CNAuthorizationStatusNotDetermined: {
        WARN << "Access to contacts not determined. Requesting access…";
        [store requestAccessForEntityType:CNEntityTypeContacts completionHandler:^(BOOL granted, NSError * _Nullable error) {
          if (error)
            WARN << QString::fromNSString(error.description);
          else if (granted)
            INFO << "User granted access to contacts";
          else
            WARN << "User denied access to contacts";
        }];
        return results;
      }
      case CNAuthorizationStatusAuthorized: {
        NSError *error = nil;
        NSPredicate *predicate = [CNContact predicateForContactsMatchingName:query->string().toNSString()];
        NSArray<CNContact *> *contacts = [store unifiedContactsMatchingPredicate:predicate
                                                keysToFetch:@[CNContactGivenNameKey,
                                                              CNContactFamilyNameKey,
                                                              CNContactEmailAddressesKey,
                                                              CNContactPhoneNumbersKey] error:&error];
        if (error) {
            WARN << QString::fromNSString(error.description);
            return results;
        }

        static const QRegularExpression re(R"R(_\$!<|>!\$_)R");

        for (CNContact *contact in contacts) {
          auto identifier = QString::fromNSString(contact.identifier);
          auto fullname = QString("%1 %2").arg(QString::fromNSString(contact.givenName),
                                               QString::fromNSString(contact.familyName));

          vector<Action> actions;

//          NSArray<CNLabeledValue<CNPhoneNumber *> *> *phoneNumbers = contact.phoneNumbers;
          for (CNLabeledValue<CNPhoneNumber *> *phoneNumber in contact.phoneNumbers) {
            auto label = QString::fromNSString(phoneNumber.label);
            label.remove(re);
            auto number = QString::fromNSString(phoneNumber.value.stringValue);


            results.emplace_back(StandardItem::make(
                identifier + "phone" + label,
                number,
                tr("Phone number '%1' of %2").arg(label, fullname),
                {"qfip:/System/Applications/Contacts.app"},
                {
                    {"copy", tr("Copy"), [number](){setClipboardText(number);}},
                    {"call", tr("Call"), [number](){openUrl("tel:"+number);}},
                    {"call", "iMessage", [number](){openUrl("sms:"+number);}}
                }
              ),
              (float)query->string().length()/fullname.size()
            );
          }

//          NSArray<CNLabeledValue<NSString *> *> *emailAddresses = contact.emailAddresses;
          for (CNLabeledValue<NSString *> *emailAddress in contact.emailAddresses) {
            auto label = QString::fromNSString(emailAddress.label);
            label.remove(re);
            auto mail = QString::fromNSString(emailAddress.value);
            results.emplace_back(StandardItem::make(
                identifier + "mail" + label,
                mail,
                tr("Mail address '%1' of %2").arg(label, fullname),
                {"qfip:/System/Applications/Contacts.app"},
                {
                    {"copy", tr("Copy"), [mail](){setClipboardText(mail);}},
                    {"mail", tr("Send mail"), [mail](){openUrl("mailto:"+mail);}},
                }
              ),
              (float)query->string().length()/fullname.size()
            );
          }
        }
        return results;
      }
      default:
        WARN << "Unknown CNAuthorizationStatus.";
        return results;
    }
  }
}


QWidget *Plugin::buildConfigWidget()
{
    auto l = new QLabel(tr("Apple contacts prototype. For now only phone and mail."));
    l->setWordWrap(true);
    l->setAlignment(Qt::AlignTop);
    return l;
}

