// Copyright (C) 2020-2024 Ivo Šmerek

#pragma once
#include <qt6/QtCore/QString>

class Device
{
public:
    QString id;
    QString name;
    QString type;
    bool isActive = false;
};
