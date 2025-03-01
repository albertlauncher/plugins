// Copyright (c) 2023-2025 Manuel Schneider

#pragma once
#include <albert/item.h>
#include <QCoreApplication>

class VpnConnectionItem : public albert::Item
{
    Q_DECLARE_TR_FUNCTIONS(VpnConnectionItem)

public:

    enum class State {
        Invalid,
        Disconnected,
        Connecting,
        Connected,
        Disconnecting
    } ;

    static QString stateString(State state);

    QString subtext() const override;
    QStringList iconUrls() const override;
    QString inputActionText() const override;

    State state() const;
    void setState(State state);

private:

    State state_;
};
