// Copyright (c) 2023 Manuel Schneider

#include "imageprovider.h"

ImageProvider::ImageProvider() : QQuickImageProvider(QQuickImageProvider::Pixmap) {}

QPixmap ImageProvider::requestPixmap(const QString &id, QSize *size, const QSize &requestedSize)
{
    return icon_provider.getPixmap(id.split(','), size, requestedSize);
}

void ImageProvider::clearCache()
{
    icon_provider.clearCache();
}
