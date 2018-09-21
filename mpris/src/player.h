// Copyright (C) 2016-2017 Martin Buergmann

#pragma once

#include <QString>

namespace MPRIS {

class Player
{
public:
    Player(const QString& busid, const QString& name, bool canRaise)
        : busId_(busid), name_(name), canRaise_(canRaise) {}

    const QString& name() const { return name_; }
    const QString& busId() const { return busId_; }
    bool canRaise() const { return canRaise_; }

private:

    QString busId_;
    QString name_;
    bool canRaise_;
};

} // namespace MPRIS
