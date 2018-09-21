// Copyright (C) 2014-2018 Manuel Schneider

#include "command.h"
#include <QtDBus/QDBusConnection>
#include <QtDBus/QDBusMessage>
#include <QtDBus/QDBusInterface>
#include <QDebug>
#include "item.h"

/** ***************************************************************************/
MPRIS::Command::Command(const QString &label, const QString &title, const QString &subtext, const QString &method, QString iconpath)
    : label_(label), title_(title), subtext_(subtext), method_(method), iconpath_(iconpath) {
}



/** ***************************************************************************/
QString& MPRIS::Command::getIconPath() {
    return iconpath_;
}



/** ***************************************************************************/
MPRIS::Command &MPRIS::Command::applicableWhen(const char* path, const char *property, const QVariant expectedValue, bool positivity) {
    path_ = path;
    property_ = property;
    expectedValue_ = expectedValue;
    positivity_ = positivity;
    applicableCheck_ = true;
    return *this;
}



/** ***************************************************************************/
SharedItem MPRIS::Command::produceAlbertItem(Player &player) const {
    QDBusMessage msg = QDBusMessage::createMethodCall(player.busId(), "/org/mpris/MediaPlayer2", "org.mpris.MediaPlayer2.Player", method_);
    SharedItem ptr(new MPRIS::Item(player, title_, subtext_, iconpath_, msg));
    return ptr;
}



/** ***************************************************************************/
bool MPRIS::Command::isApplicable(Player &p) const {
    // Check the applicable-option if given
    if (!applicableCheck_)
        return true;

    // split DBus interface and property into seperate strings
    int splitAt = property_.lastIndexOf('.');
    QString ifaceName = property_.left(splitAt);
    QString propertyName = property_.right(property_.length() - splitAt -1);

    // Compose Get-Property-Message
    QDBusMessage mesg = QDBusMessage::createMethodCall(
                p.busId(), //"org.mpris.MediaPlayer2.rhythmbox",
                path_, //"/org/mpris/MediaPlayer2",
                "org.freedesktop.DBus.Properties",
                "Get");
    QList<QVariant> args;
    // Specify DBus interface to get the property from and the property-name
    args.append(ifaceName); //"org.mpris.MediaPlayer2.Player");
    args.append(propertyName); //"CanGoNext");
    mesg.setArguments(args);

    // Query the property
    QDBusMessage reply = QDBusConnection::sessionBus().call(mesg);

    // Check if the result is as expected
    if ( reply.type() != QDBusMessage::ReplyMessage ){
        qWarning() << "Error while querying the property 'PlaybackStatus'";
        return true;
    }

    if ( reply.arguments().empty() ) {
        qWarning() << "Reply query 'PlaybackStatus' is empty";
        return true;
    }

    return (reply.arguments().at(0).value<QDBusVariant>().variant() == expectedValue_) == positivity_;
}



/** ***************************************************************************/
QString& MPRIS::Command::getLabel() {
    return label_;
}



/** ***************************************************************************/
QString& MPRIS::Command::getMethod() {
    return method_;
}



/** ***************************************************************************/
QString& MPRIS::Command::getTitle() {
    return title_;
}

