// Copyright (c) 2024-2025 Manuel Schneider

#pragma once
#include <map>
#include <QStringList>
class QPalette;

void writePalette(const QPalette &palette, const QString &path);
QPalette readPalette(const QString &path);
std::map<QString, QString> findPalettes(const QStringList &data_dirs);

