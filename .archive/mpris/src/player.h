// Copyright (c) 2022 Manuel Schneider

#pragma once

#include <QString>

struct Player
{
    QString busId;
    QString name;
    bool canRaise;
};

//class Player
//{
//public:
//    Player(const QString& busid, const QString& name, bool canRaise)
//        : busId_(busid), name_(name), canRaise_(canRaise) {}

//    const QString& name() const { return name_; }
//    const QString& busId() const { return busId_; }
//    bool canRaise() const { return canRaise_; }

//private:

//    QString busId_;
//    QString name_;
//    bool canRaise_;
//};
