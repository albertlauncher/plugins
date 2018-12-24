// Copyright (C) 2016-2017 Martin Buergmann

#include "item.h"
#include "albert/util/standardactions.h"
#include <QDBusConnection>


/** ***************************************************************************/
MPRIS::Item::Item(Player &p, const QString &title, const QString &subtext, const QString &iconPath, const QDBusMessage &msg)
    : iconPath_(iconPath), message_(msg) {
    if (title.contains("%1"))
        text_ = title.arg(p.name());
    else
        text_ = title;
    if (subtext.contains("%1"))
        subtext_ = subtext.arg(p.name());
    else
        subtext_ = subtext;
    actions_.push_back(std::make_shared<Core::FuncAction>(subtext_, [this](){ QDBusConnection::sessionBus().send(message_); }));
    if (p.canRaise()) {
        actions_.push_back(std::make_shared<Core::FuncAction>("Raise Window", [&p](){
            QString busid = p.busId();
            QDBusMessage raise = QDBusMessage::createMethodCall(busid, "/org/mpris/MediaPlayer2", "org.mpris.MediaPlayer2", "Raise");
            if (!QDBusConnection::sessionBus().send(raise)) {
                qWarning("Error calling raise method on dbus://%s", busid.toStdString().c_str());
            }
        }));
    }
    id_ = "extension.mpris.item:%1.%2";
    id_ = id_.arg(p.busId()).arg(msg.member());
}



/** ***************************************************************************/
MPRIS::Item::~Item() {

}



/** ***************************************************************************/
vector<shared_ptr<Action>> MPRIS::Item::actions() {
    return actions_;
}

