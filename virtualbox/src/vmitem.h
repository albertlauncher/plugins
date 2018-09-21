// Copyright (C) 2016-2017 Martin Buergmann

#pragma once
#include "core/item.h"

namespace VirtualBox {

class VMItem : public Core::Item
{
public:
    VMItem(const QString &name,
           const QString &uuid,
           int &mainAction,
           const std::vector<std::shared_ptr<Core::Action>> actions,
           const QString &state);


    /*
     * Implementation of AlbertItem interface
     */

    QString id() const override { return idstring_; }
    QString text() const override { return name_; }
    QString subtext() const override;
    QString iconPath() const override { return iconPath_; }
    std::vector<std::shared_ptr<Core::Action>> actions() override { return actions_; }

    /*
     * Item specific members
     */

    static QString iconPath_;
    static const int VM_START;
    static const int VM_PAUSE;
    static const int VM_RESUME;
    static const int VM_STATE_CHANGING;
    static const int VM_DIFFERENT;

private:
    QString name_;
    QString uuid_;
    QString idstring_;
    std::vector<std::shared_ptr<Core::Action>> actions_;
    int mainAction_;
};

} // namespace VirtualBox
