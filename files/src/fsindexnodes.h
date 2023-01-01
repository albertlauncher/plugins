// Copyright (c) 2022 Manuel Schneider

#pragma once
#include <QFileSystemWatcher>
#include <QFutureWatcher>
#include <QRegularExpression>
#include <QTimer>
#include <set>

class IndexFileItem;
class AbstractFileItem;
class QMimeType;


enum class PatternType { Include, Exclude };


struct NameFilter {
    NameFilter(QRegularExpression regex, PatternType type);
    explicit NameFilter(const QString &pattern);
    QRegularExpression regex;
    PatternType type;
};


struct IndexSettings
{
    std::vector<NameFilter> name_filters;
    std::vector<QRegularExpression> mime_filters;
    uint8_t max_depth;
    bool index_hidden_files;
    bool follow_symlinks;
    bool forced;
    // uninformed update (scan)
    // traverse the entire tree anyway, because child dirs may have been modified
    bool scan_mode = true; // Todo use this for future filesystem watches(false)
};


class DirNode
{
public:
    virtual ~DirNode();

    static std::shared_ptr<DirNode> make(QString name,
                                         const std::shared_ptr<DirNode>& parent = nullptr,
                                         uint64_t mdate = 0);
    static std::shared_ptr<DirNode> fromJson(const QJsonObject&, const std::shared_ptr<DirNode>& parent);
    QJsonObject toJson() const;

    void removeChildren();
    void update(const std::shared_ptr<DirNode>& shared_this,
                const bool &abort,
                std::function<void(const QString&)> &status,
                const IndexSettings &settings,
                std::set<QString> &indexed_dirs,
                uint depth);

    virtual QString path() const;
    virtual QString filePath() const;
    virtual QString relativeFilePath() const; // relative to root dir. note: '/' prepended.

    void items(std::vector<std::shared_ptr<AbstractFileItem>>&) const;
    void nodes(std::vector<std::shared_ptr<DirNode>>&) const;
    std::shared_ptr<DirNode> node(const QString &relative_path) const;

    static QMimeType dirMimeType();

protected:
    DirNode(QString name, const std::shared_ptr<DirNode>& parent = nullptr, uint64_t mdate = 0);
    DirNode(DirNode&&) = delete;
    DirNode(const DirNode&) = delete;
    DirNode &operator=(DirNode&&) = delete;
    DirNode &operator=(const DirNode&) = delete;

    const std::shared_ptr<DirNode> parent_;
    QString name_;
    uint32_t mdate_;
    std::vector<std::shared_ptr<DirNode>> children_;
    std::vector<std::shared_ptr<IndexFileItem>> items_;
};


class RootNode: public DirNode
{
public:
    ~RootNode();

    static std::shared_ptr<RootNode> make(QString name);
    static std::shared_ptr<RootNode> fromJson(const QJsonObject&);
    QJsonObject toJson() const;

    QString path() const override;
    QString filePath() const override;
    QString relativeFilePath() const override;
private:
    RootNode(QString filePath);
    RootNode(RootNode&&) = delete;
    RootNode(const RootNode&) = delete;
    RootNode &operator=(RootNode&&) = delete;
    RootNode &operator=(const RootNode&) = delete;
    QString path_;
};
