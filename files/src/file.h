// Copyright (C) 2014-2018 Manuel Schneider

#pragma once
#include <QString>
#include <QMimeType>
#include <map>
#include <vector>
#include "albert/indexitem.h"

namespace Files {

class File : public Core::IndexItem
{
public:

    QString id() const override;
    QString text() const override;
    QString subtext() const override;
    QString completion() const override;
    QString iconPath() const override;
    std::vector<Core::IndexItem::IndexString> indexStrings() const override;
    std::vector<std::shared_ptr<Core::Action>> actions() override;

    static std::vector<std::shared_ptr<Core::Action>> buildFileActions(const QString &filePath);

    /** Return the filename of the file */
    virtual QString name() const = 0;

    /** Return the path exclusive the filename of the file */
    virtual QString path() const = 0;

    /** Return the path inclusive the filename of the file */
    virtual QString filePath() const = 0;

    /** Return the mimetype of the file */
    virtual const QMimeType &mimetype() const = 0;

};

}
