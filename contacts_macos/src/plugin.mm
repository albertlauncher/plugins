// Copyright (c) 2023 Manuel Schneider

#include "plugin.h"
#include <Contacts/Contacts.h>
ALBERT_LOGGING
using namespace std;
using namespace albert;


vector<RankItem> Plugin::handleGlobalQuery(const GlobalQuery &query) const
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
        NSPredicate *predicate = [CNContact predicateForContactsMatchingName:query.string().toNSString()];
        NSArray<CNContact *> *contacts = [store unifiedContactsMatchingPredicate:predicate
                                                keysToFetch:@[CNContactGivenNameKey,
                                                              CNContactFamilyNameKey,
                                                              CNContactEmailAddressesKey,
                                                              CNContactPhoneNumbersKey] error:&error];
        if (error) {
            WARN << QString::fromNSString(error.description);
            return results;
        }

        for (CNContact *contact in contacts) {
          auto identifier = QString::fromNSString(contact.identifier);
          auto fullname = QString("%1 %2").arg(QString::fromNSString(contact.givenName),
                                               QString::fromNSString(contact.familyName));

          vector<Action> actions;
          NSArray<CNLabeledValue<CNPhoneNumber *> *> *phoneNumbers = contact.phoneNumbers;
          for (CNLabeledValue<CNPhoneNumber *> *phoneNumber in phoneNumbers) {
            auto label = QString::fromNSString(phoneNumber.label);
            auto number = QString::fromNSString(phoneNumber.value.stringValue);
            results.emplace_back(StandardItem::make(
                identifier + "phone" + label,
                number,
                QString("%1 of %2").arg(label, fullname),
                {"qfip:/System/Applications/Contacts.app"},
                {
                  {"copy", "Copy", [number](){setClipboardText(number);}},
                  {"call", "Call", [number](){openUrl("tel:"+number);}},
                  {"call", "iMessage", [number](){openUrl("sms:"+number);}}
                }
              ),
              (double)query.string().length()/(double)fullname.size()*RankItem::MAX_SCORE
            );
          }

          NSArray<CNLabeledValue<NSString *> *> *emailAddresses = contact.emailAddresses;
          for (CNLabeledValue<NSString *> *emailAddress in emailAddresses) {
            auto label = QString::fromNSString(emailAddress.label);
            auto mail = QString::fromNSString(emailAddress.value);
            results.emplace_back(StandardItem::make(
                identifier + "mail" + label,
                mail,
                QString("%1 of %2").arg(label, fullname),
                {"qfip:/System/Applications/Contacts.app"},
                {
                  {"copy", "Copy", [mail](){setClipboardText(mail);}},
                  {"mail", "Send mail", [mail](){openUrl("mailto:"+mail);}},
                }
              ),
              (double)query.string().length()/(double)fullname.size()*RankItem::MAX_SCORE
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
