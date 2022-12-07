// Copyright (c) 2022 Manuel Schneider

#include <QtDBus/QDBusConnection>
#include <QtDBus/QDBusMessage>
#include <QtDBus/QDBusInterface>
#include <QDebug>
#include "command.h"
#include "player.h"
using namespace albert;


Command::Command(const QString &label, const QString &title, const QString &subtext, const QString &method, QStringList icon)
    : label_(label), title_(title), subtext_(subtext), method_(method), icon_(icon)
{

}


QStringList& Command::getIcon()
{
    return icon_;
}


Command &Command::applicableWhen(const char* path, const char *property, const QVariant expectedValue, bool positivity)
{
    path_ = path;
    property_ = property;
    expectedValue_ = expectedValue;
    positivity_ = positivity;
    applicableCheck_ = true;
    return *this;
}


albert::SStdItem Command::produceAlbertItem(Player &player) const
{
    QDBusMessage msg = QDBusMessage::createMethodCall(player.busId, "/org/mpris/MediaPlayer2", "org.mpris.MediaPlayer2.Player", method_);

    auto item = makeSStdItem(QString("mpris:%1.%2").arg(player.busId, msg.member()),
                             icon_,
                             title_.contains("%1") ? title_.arg(player.name): title_,
                             subtext_.contains("%1") ? subtext_.arg(player.name): subtext_);

    item->addAction(makeFuncAction(subtext_, [msg](){ QDBusConnection::sessionBus().send(msg); }));

    if (player.canRaise) {
        item->addAction(makeFuncAction("Raise Window", [&player](){
            QString busid = player.busId;
            QDBusMessage raise = QDBusMessage::createMethodCall(busid, "/org/mpris/MediaPlayer2", "org.mpris.MediaPlayer2", "Raise");
            if (!QDBusConnection::sessionBus().send(raise)) {
                qWarning("Error calling raise method on dbus://%s", busid.toStdString().c_str());
            }
        }));
    }

    return item;
}


bool Command::isApplicable(Player &p) const
{
    // Check the applicable-option if given
    if (!applicableCheck_)
        return true;

    // split DBus interface and property into seperate strings
    int splitAt = property_.lastIndexOf('.');
    QString ifaceName = property_.left(splitAt);
    QString propertyName = property_.right(property_.length() - splitAt -1);

    // Compose Get-Property-Message
    QDBusMessage mesg = QDBusMessage::createMethodCall(
                p.busId, //"org.mpris.MediaPlayer2.rhythmbox",
                path_, //"/org/mpris/MediaPlayer2",
                "org.freedesktop.DBus.Properties",
                "Get");
    QList<QVariant> args;
    // Specify DBus interface to get the property from and the property-name
    args.append(ifaceName); //"org.mpris.MediaPlayer2.Player");
    args.append(propertyName); //"CanGoNext");
    mesg.setArguments(args);

    // QueryExecution the property
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


QString& Command::getLabel()
{
    return label_;
}


QString& Command::getMethod()
{
    return method_;
}


QString& Command::getTitle()
{
    return title_;
}

