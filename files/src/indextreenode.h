// Copyright (C) 2014-2018 Manuel Schneider

#pragma once
#include <QDateTime>
#include <QDebug>
#include <QDir>
#include <QString>
#include <QMimeDatabase>
#include <QRegularExpression>
#include <memory>
#include <vector>
#include <set>
#include "indexfile.h"

namespace Files {

class Visitor;
class IndexSettings;

enum class PatternType {
    Include,
    Exclude
};

struct IgnoreEntry {
    IgnoreEntry(QRegularExpression regex, PatternType type) : regex(regex), type(type) {}
    QRegularExpression regex;
    PatternType type;
};

class IndexTreeNode final : public std::enable_shared_from_this<IndexTreeNode>
{
public:

    IndexTreeNode();
    IndexTreeNode(const IndexTreeNode & other);
    IndexTreeNode(QString name, QDateTime lastModified, std::shared_ptr<IndexTreeNode> parent = std::shared_ptr<IndexTreeNode>());
    IndexTreeNode(QString name, std::shared_ptr<IndexTreeNode> parent = std::shared_ptr<IndexTreeNode>());
    ~IndexTreeNode();

    void accept(Visitor &visitor);

    void removeDownlinks();

    QString path() const;

    void update(const bool &abort, IndexSettings indexSettings);

    QJsonObject serialize();
    void deserialize(const QJsonObject &, std::shared_ptr<IndexTreeNode> parent = std::shared_ptr<IndexTreeNode>());

    const std::vector<std::shared_ptr<IndexFile> > &items() const;

private:

    void updateRecursion(const bool &abort,
                         const QMimeDatabase &mimeDatabase,
                         const IndexSettings &indexSettings,
                         std::set<QString> *indexedDirs,
                         const std::vector<IgnoreEntry> &ignoreEntries = std::vector<IgnoreEntry>());

    std::shared_ptr<IndexTreeNode> parent;
    std::vector<std::shared_ptr<IndexTreeNode>> children;
    QString name;
    QDateTime lastModified;
    std::vector<std::shared_ptr<Files::IndexFile>> items_;

    static constexpr const char* IGNOREFILE = ".albertignore";
};


/** ***********************************************************************************************/
class IndexSettings
{
public:
    const std::vector<QRegExp> &filters() const;
    void setFilters(std::vector<QRegExp> value);
    void setFilters(QStringList value);

    bool indexHidden() const;
    void setIndexHidden(bool value);

    bool followSymlinks() const;
    void setFollowSymlinks(bool value);

    bool forceUpdate() const;
    void setForceUpdate(bool value);

    bool fuzzy() const;
    void setFuzzy(bool value);


private:

    std::vector<QRegExp> mimefilters_;
    bool indexHidden_ = false;
    bool followSymlinks_ = false;
    bool fuzzy_ = false;
    bool forceUpdate_ = false; // Ignore lastModified, force update

};


/** ***********************************************************************************************/
class Visitor {
public:
    virtual ~Visitor() { }
    virtual void visit(IndexTreeNode *) = 0;
};

}
