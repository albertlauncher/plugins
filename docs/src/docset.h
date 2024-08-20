// Copyright (c) 2022-2024 Manuel Schneider

#pragma once
#include <QString>
#include <albert/indexitem.h>
#include <vector>

class Docset
{
public:

    Docset(QString name, QString title, QString source_id, QString icon_path);

    void createIndexItems(std::vector<albert::IndexItem> &results) const;

    bool isInstalled() const;

    const QString name;
    const QString title;
    const QString source_id;
    const QString icon_path;
    QString path;  // not downloaded yet if null

};
