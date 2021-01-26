// Copyright (C) 2020-2021 Ivo Šmerek

#include <QString>

namespace Spotify {

    class Device {

    public:
        Device() = default;
        ~Device() = default;

        QString id;
        QString name;
        QString type;
        bool isActive = false;
    };
}