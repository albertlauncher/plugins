// Copyright (C) 2016-2017 Martin Buergmann

#pragma once
#include <QDBusMessage>
#include <QVariant>
#include <vector>
#include "util/standarditem.h"
#include "player.h"
using std::vector;
using std::shared_ptr;
using Core::Action;

namespace MPRIS {

class Item final : public Core::Item
{
public:

    Item(Player &p, const QString& title, const QString& subtext, const QString& iconPath, const QDBusMessage& msg);
    ~Item();

    QString id() const override { return id_; }
    QString text() const override { return text_; }
    QString subtext() const override { return subtext_; }
    QString iconPath() const override { return iconPath_; }
    vector<shared_ptr<Action>> actions() override;

private:

    QString id_;
    QString text_;
    QString subtext_;
    QString iconPath_;
    QDBusMessage message_;
    vector<shared_ptr<Action>> actions_;

};

}
