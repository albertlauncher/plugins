// Copyright (C) 2016-2017 Martin Buergmann

#pragma once

#include <QString>
#include "vmitem.h"
#include "VirtualBox_XPCOM.h"

namespace VirtualBox {

class VM
{
public:
    VM(IMachine *machine);
    virtual ~VM();
    VMItem* produceItem() const;
    bool startsWith(QString other) const;
    const QString &uuid() const { return uuid_; }
    const QString &name() const { return name_; }

private:
    IMachine* machine_;
    QString name_;
    QString uuid_;
    mutable QString state_;
};

} // namespace VirtualBox

