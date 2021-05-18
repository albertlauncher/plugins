// Copyright (C) 2020-2021 Ivo Šmerek

#include <QString>

namespace Spotify {

class Track {

public:
    Track() = default;
    ~Track() = default;

    QString id;
    QString name;
    QString artists;
    QString albumId;
    QString albumName;
    QString uri;
    QString imageUrl;
    bool isExplicit = false;
};
}
