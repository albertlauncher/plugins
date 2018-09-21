// Copyright (C) 2014-2018 Manuel Schneider

#include <QDir>
#include <QFileInfo>
#include "indexfile.h"
#include "indextreenode.h"
using namespace std;



/** ***************************************************************************/
Files::IndexFile::IndexFile(QString name, const shared_ptr<Files::IndexTreeNode> &pathNode, QMimeType mimetype)
    : name_(name), pathNode_(pathNode), mimetype_(mimetype) { }


/** ***************************************************************************/
QString Files::IndexFile::name() const {
    return name_;
}


/** ***************************************************************************/
QString Files::IndexFile::path() const {
    return pathNode_->path();
}


/** ***************************************************************************/
QString Files::IndexFile::filePath() const {
    return QDir(pathNode_->path()).filePath(name_);
}

/** ***************************************************************************/
const QMimeType &Files::IndexFile::mimetype() const {
    return mimetype_;
}
