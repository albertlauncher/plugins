// Copyright (c) 2023 Manuel Schneider

#pragma once
#include <QQuickImageProvider>
#include "albert/util/iconprovider.h"

class ImageProvider : public QQuickImageProvider
{
public:
    ImageProvider();
    QPixmap requestPixmap(const QString &id, QSize *size, const QSize &requestedSize) override;
    void clearCache();

private:
    albert::IconProvider icon_provider;

};
