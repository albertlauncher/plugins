// Copyright (c) 2023-2024 Manuel Schneider

#include "imageprovider.h"
#include <albert/iconprovider.h>

ImageProvider::ImageProvider() : QQuickImageProvider(QQuickImageProvider::Pixmap) {}

QPixmap ImageProvider::requestPixmap(const QString &id, QSize *size, const QSize &requestedSize)
{
    // return icon_provider.getPixmap(id.split(','), size, requestedSize);
    auto pm = albert::pixmapFromUrls(id.split(','), requestedSize);
    *size = pm.size();
    return pm;
}
