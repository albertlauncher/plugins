// Copyright (C) 2014-2018 Manuel Schneider

#include <QDir>
#include <QFileInfo>
#include "standardfile.h"
using namespace std;


/** ***************************************************************************/
Files::StandardFile::StandardFile(QString path, QMimeType mimetype)
    : mimetype_(mimetype){
    QFileInfo fileInfo(path);
    name_ = fileInfo.fileName();
    path_ = fileInfo.canonicalPath();
}


/** ***************************************************************************/
QString Files::StandardFile::name() const {
    return name_;
}


/** ***************************************************************************/
QString Files::StandardFile::path() const {
    return path_;
}


/** ***************************************************************************/
QString Files::StandardFile::filePath() const {
    return QDir(path_).filePath(name_);
}

/** ***************************************************************************/
const QMimeType &Files::StandardFile::mimetype() const {
    return mimetype_;
}
