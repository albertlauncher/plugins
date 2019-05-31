// Copyright (C) 2014-2018 Manuel Schneider

#include <QJsonArray>
#include <QJsonObject>
#include "indextreenode.h"
#include "indexfile.h"
using namespace std;


/**************************************************************************************************/
Files::IndexTreeNode::IndexTreeNode() { }



/**************************************************************************************************/
Files::IndexTreeNode::IndexTreeNode(QString name, QDateTime lastModified, shared_ptr<IndexTreeNode> parent)
    : enable_shared_from_this(), parent(parent), name(name), lastModified(lastModified) { }


/**************************************************************************************************/
Files::IndexTreeNode::IndexTreeNode(QString name, shared_ptr<IndexTreeNode> parent)
    : IndexTreeNode(name, QDateTime::fromMSecsSinceEpoch(0), parent) { }


/**************************************************************************************************/
Files::IndexTreeNode::IndexTreeNode(const IndexTreeNode &other)
    : enable_shared_from_this(),
      children(other.children),
      name(other.name),
      lastModified(other.lastModified),
      items_(other.items_) { }


/**************************************************************************************************/
Files::IndexTreeNode::~IndexTreeNode() {
    removeDownlinks();
}


/**************************************************************************************************/
void Files::IndexTreeNode::accept(Visitor &visitor) {
    visitor.visit(this);
    for ( auto & child : children )
        child->accept(visitor);
}


/**************************************************************************************************/
void Files::IndexTreeNode::removeDownlinks() {
    for ( shared_ptr<IndexTreeNode> & child : children )
        child->removeDownlinks();
    children.clear();
    items_.clear();
}


/**************************************************************************************************/
QString Files::IndexTreeNode::path() const {
    return ( parent ) ? QDir(parent->path()).filePath(name) : name;
}


/**************************************************************************************************/
void Files::IndexTreeNode::update(const bool  &abort, IndexSettings indexSettings) {
    set<QString> indexedDirs;
    QMimeDatabase mimeDatabase;
    updateRecursion(abort, mimeDatabase, indexSettings, &indexedDirs);
}


/**************************************************************************************************/
QJsonObject Files::IndexTreeNode::serialize(){

    QJsonObject jsonNode;

    jsonNode.insert("name", this->name);
    jsonNode.insert("lastmodified", this->lastModified.toMSecsSinceEpoch());

    QJsonArray itemArray;
    for ( const shared_ptr<IndexFile> &file : items_ ) {
        QJsonObject jsonFile;
        jsonFile.insert("name", file->name());
        jsonFile.insert("mimetype", file->mimetype().name());
        itemArray.push_back(jsonFile);
    }
    jsonNode.insert("items", itemArray);

    QJsonArray nodeArray;
    for ( const shared_ptr<IndexTreeNode> &childNode : children )
        nodeArray.push_back(childNode->serialize());
    jsonNode.insert("children", nodeArray);

    return jsonNode;
}


/**************************************************************************************************/
void Files::IndexTreeNode::deserialize(const QJsonObject &object, shared_ptr<IndexTreeNode> parent) {

    this->parent = parent;
    name = object["name"].toString();
    lastModified = QDateTime::fromMSecsSinceEpoch(object["lastmodified"].toVariant().toLongLong());

    for (const QJsonValueRef child : object["children"].toArray()) {
        children.push_back(make_shared<IndexTreeNode>()); // Invalid node
        children.back()->deserialize(child.toObject(), shared_from_this());
    }

    for (const QJsonValueRef item : object["items"].toArray()){
        const QJsonObject &object = item.toObject();
        items_.push_back(make_shared<IndexFile>(
                             object["name"].toString(),
                             shared_from_this(),
                             QMimeDatabase().mimeTypeForName(object["mimetype"].toString())));
    }
}


/**************************************************************************************************/
const std::vector<std::shared_ptr<Files::IndexFile> > &Files::IndexTreeNode::items() const {
    return items_;
}


/**************************************************************************************************/
void Files::IndexTreeNode::updateRecursion(const bool &abort,
                                           const QMimeDatabase &mimeDatabase,
                                           const IndexSettings &indexSettings,
                                           std::set<QString> *indexedDirs,
                                           const vector<IgnoreEntry> &ignoreEntries){
    const QFileInfo fileInfo(path());

    // Skip if this dir has already been indexed (loop detection)
    if ( indexedDirs->count(fileInfo.canonicalFilePath()) )
        return;

    // Read the ignore file, see http://doc.qt.io/qt-5/qregexp.html#wildcard-matching
    vector<IgnoreEntry> localIgnoreEntries = ignoreEntries;
    QFile file(QDir(fileInfo.filePath()).filePath(IGNOREFILE));
    if ( file.open(QIODevice::ReadOnly | QIODevice::Text) ) {
        QTextStream in(&file);
        while ( !in.atEnd() ) {
            QString pattern = QDir::cleanPath(in.readLine());

            if ( pattern.isEmpty() || pattern.startsWith("#") )
                continue;

            // Replace ** and * by their regex analogons
            pattern.replace(QRegularExpression("(?<!\\*)\\*(?!\\*)"), "[^\\/]*");
            pattern.replace(QRegularExpression("\\*{2,}"), ".*");

            // Determine pattern type
            PatternType patternType = PatternType::Exclude;
            if ( pattern.startsWith('!') ) {
                patternType = PatternType::Include;
                pattern = pattern.mid(1, -1);
            }

            // Respect files beginning with excalmation mark
            if ( pattern.startsWith("\\!") )
                pattern = pattern.mid(1, -1);

            if ( pattern.startsWith("/") ) {
                pattern = QString("^%1$").arg(QDir(fileInfo.filePath()).filePath(pattern.mid(1, -1)));
                localIgnoreEntries.emplace_back(QRegularExpression(pattern), patternType);
            } else {
                pattern = QString("/%1$").arg(pattern);
                localIgnoreEntries.emplace_back(QRegularExpression(pattern), patternType);
            }
        }
        file.close();
    }


    if ( lastModified < fileInfo.lastModified() || indexSettings.forceUpdate() ) {

        indexedDirs->insert(fileInfo.canonicalFilePath());

        QString absFilePath = fileInfo.absoluteFilePath();
        qDebug() << "Indexing directory " << absFilePath;

        lastModified = fileInfo.lastModified();

        // Drop nonexistant child nodes
        decltype(children)::iterator childIt = children.begin();
        while ( childIt != children.end() )
            if ( !QFile::exists((*childIt)->path()) ) {
                (*childIt)->removeDownlinks();
                childIt = children.erase(childIt);
            } else
                ++childIt;


        // Handle the directory contents
        items_.clear();
        auto filters = QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot | QDir::Hidden;
        for ( const QFileInfo &fileInfo : QDir(absFilePath).entryInfoList(filters, QDir::Name) ){

            // Skip check if this file should be excluded
            PatternType patternType = PatternType::Include;
            for ( const IgnoreEntry &ignoreEntry : localIgnoreEntries )
                if ( ignoreEntry.regex.match(fileInfo.filePath()).hasMatch() )
                    patternType = ignoreEntry.type;

            // Add directories as nodes
            if ( fileInfo.isDir() ) {

                // Get the place where the item were if would exist in log(n) time
                decltype(children)::iterator lb = lower_bound(
                            children.begin(), children.end(), fileInfo.fileName(),
                            [&](const shared_ptr<IndexTreeNode> & child, QString name) {
                    return child->name < name;
                });

                if ( lb == children.end() || (*lb)->name != fileInfo.fileName() ) {
                    // If does not exist and is valid insert
                    if (!( patternType == PatternType::Exclude  // Skip check if this file should be excluded
                           || ( fileInfo.isSymLink() && !indexSettings.followSymlinks() )  // Skip if symlink and we should skip these
                           || ( fileInfo.isHidden() && !indexSettings.indexHidden() )))  // Skip if hidden and we should skip these
                        children.insert(lb, make_shared<IndexTreeNode>(fileInfo.fileName(), shared_from_this()));
                }
                else if (patternType == PatternType::Exclude  // Skip check if this file should be excluded
                         || ( fileInfo.isSymLink() && !indexSettings.followSymlinks() )  // Skip if symlink and we should skip these
                         || ( fileInfo.isHidden() && !indexSettings.indexHidden() )) {  // Skip if hidden and we should skip these
                    // If does exist and is invalid remove node
                    (*lb)->removeDownlinks();
                    children.erase(lb);
                }
            }

            // Add entries as items
            const QMimeType mimetype = mimeDatabase.mimeTypeForFile(fileInfo.filePath());
            const QString mimeName = mimetype.name();

            // If the entry is valid and a mime filter matches add it to the items
            if (!( patternType == PatternType::Exclude     // Skip check if this file should be excluded
                  || ( fileInfo.isHidden() && !indexSettings.indexHidden()) )  // Skip if hidden and we should skip these
                  && any_of(indexSettings.filters().begin(), indexSettings.filters().end(),
                            [&](const QRegExp &re){ return re.exactMatch(mimeName); }) )
                items_.push_back(make_shared<Files::IndexFile>(fileInfo.fileName(),
                                                               shared_from_this(),
                                                               mimetype));
        }
    }

    // Recursively check all childnodes too
    for ( auto it = children.begin(); !abort && it < children.end(); ++it)
        (*it)->updateRecursion(abort, mimeDatabase, indexSettings, indexedDirs, localIgnoreEntries);

}


/**************************************************************************************************/
/**************************************************************************************************/


const std::vector<QRegExp> &Files::IndexSettings::filters() const {
    return mimefilters_;
}

void Files::IndexSettings::setFilters(std::vector<QRegExp> value) {
    forceUpdate_= true;
    mimefilters_= value;
}

void Files::IndexSettings::setFilters(QStringList value) {
    forceUpdate_= true;
    mimefilters_.clear();
    for ( const QString &re : value )
        mimefilters_.emplace_back(re, Qt::CaseInsensitive, QRegExp::Wildcard);
}

bool Files::IndexSettings::indexHidden() const {
    return indexHidden_;
}

void Files::IndexSettings::setIndexHidden(bool value) {
    forceUpdate_= true;
    indexHidden_= value;
}

bool Files::IndexSettings::followSymlinks() const {
    return followSymlinks_;
}

void Files::IndexSettings::setFollowSymlinks(bool value) {
    forceUpdate_= true;
    followSymlinks_= value;
}

bool Files::IndexSettings::forceUpdate() const {
    return forceUpdate_;
}

void Files::IndexSettings::setForceUpdate(bool value) {
    forceUpdate_= value;
}

bool Files::IndexSettings::fuzzy() const {
    return fuzzy_;
}

void Files::IndexSettings::setFuzzy(bool value) {
    fuzzy_= value;
}


/**************************************************************************************************/
/**************************************************************************************************/
