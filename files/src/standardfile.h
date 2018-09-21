// Copyright (C) 2014-2018 Manuel Schneider

#pragma once
#include <QMimeType>
#include <map>
#include <memory>
#include <vector>
#include "file.h"

namespace Files {

class StandardFile : public File
{
public:

    StandardFile(QString path, QMimeType mimetype);

    QString name() const override;
    QString path() const override;
    QString filePath() const override;
    const QMimeType &mimetype() const override;

protected:

    QString name_;
    QString path_;
    QMimeType mimetype_;

};

}
