// Copyright (C) 2014-2018 Manuel Schneider

#pragma once
#include <QMimeType>
#include <map>
#include <memory>
#include <vector>
#include "file.h"

namespace Files {

class IndexTreeNode;

class IndexFile : public File
{
public:

    IndexFile(QString name, const std::shared_ptr<IndexTreeNode> &pathNode, QMimeType mimetype);

    QString name() const override;
    QString path() const override;
    QString filePath() const override;
    const QMimeType &mimetype() const override;

protected:


    QString name_;
    std::shared_ptr<IndexTreeNode> pathNode_;
    QMimeType mimetype_;

};

}
