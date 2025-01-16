// Copyright (C) 2020-2024 Ivo Šmerek

#pragma once
#include <qt6/QtCore/QString>

class Track
{
public:
    QString id;
    QString name;
    QString artists;
    QString albumId;
    QString albumName;
    QString uri;
    QString imageUrl;
    bool isExplicit = false;
};
