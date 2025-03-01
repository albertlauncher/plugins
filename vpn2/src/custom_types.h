// Copyright (c) 2023-2024 Manuel Schneider

#pragma once

#include <QVariantMap>


using NestedVariantMap = QMap<QString, QVariantMap>;
Q_DECLARE_METATYPE(NestedVariantMap)
