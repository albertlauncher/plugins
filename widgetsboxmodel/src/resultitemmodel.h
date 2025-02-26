// Copyright (c) 2023-2024 Manuel Schneider

#pragma once
#include <QAbstractListModel>
#include <vector>
#include <map>
namespace albert{
class Extension;
class Item;
class Query;
class RankItem;
class ResultItem;
}

enum class ItemRoles
{
    TextRole = Qt::DisplayRole,  ///< QString, The text
    SubTextRole = Qt::UserRole,  ///< QString, The subtext
    InputActionRole,             ///< QString, The tab action text
    IconUrlsRole,                ///< QStringList, Urls for icon lookup
    ActionsListRole,             ///< QStringList, List of action names
    ActivateActionRole,          ///< only used for setData. Activates items.
    // Dont change these without changing ItemsModel::roleNames
};

class ResultItemsModel : public QAbstractListModel
{
public:
    ResultItemsModel(albert::Query &query);

    QHash<int, QByteArray> roleNames() const override;

    QVariant getResultItemData(const albert::ResultItem &result_item, int role) const;
    void activate(albert::Query *q, uint i, uint a);

protected:
    albert::Query &query_;
    mutable std::map<const albert::ResultItem *, QStringList> actions_cache_;
};


class MatchItemsModel : public ResultItemsModel
{
public:
    using ResultItemsModel::ResultItemsModel;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role) override;
    // void fetchMore(const QModelIndex &parent) override;
    // bool canFetchMore(const QModelIndex &) const override;
};


class FallbackItemsModel : public ResultItemsModel
{
public:
    using ResultItemsModel::ResultItemsModel;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role) override;
};






// class ResultsModel final : public QAbstractListModel
// {
// public:
//     ResultsModel(, QObject *parent = nullptr); // important for qml cppownership

//     QHash<int, QByteArray> roleNames() const override;
//     int rowCount(const QModelIndex &parent = QModelIndex()) const override;
//     QVariant data(const QModelIndex &index, int role) const override;
//     void fetchMore(const QModelIndex &parent) override;
//     bool canFetchMore(const QModelIndex &parent) const override;

//     void add(albert::Extension*, std::vector<std::shared_ptr<albert::Item>>&&);

//     void add(std::vector<std::pair<albert::Extension*,std::shared_ptr<albert::Item>>>::iterator begin,
//              std::vector<std::pair<albert::Extension*,std::shared_ptr<albert::Item>>>::iterator end);

//     void add(std::vector<std::pair<albert::Extension*,albert::RankItem>>::iterator begin,
//              std::vector<std::pair<albert::Extension*,albert::RankItem>>::iterator end);

//     QAbstractListModel *buildActionsModel(uint i) const;

//     void activate(albert::Query *q, uint i, uint a);

// private:
//     std::vector<std::pair<albert::Extension*, std::shared_ptr<albert::Item>>> items;
//     mutable std::map<std::pair<albert::Extension*,albert::Item*>, QStringList> actionsCache;

// };

// class ItemsModel final : public QAbstractListModel
// {
// public:
//     ItemsModel(QObject *parent = nullptr); // important for qml cppownership

//     QHash<int, QByteArray> roleNames() const override;
//     int rowCount(const QModelIndex &parent = QModelIndex()) const override;
//     QVariant data(const QModelIndex &index, int role) const override;
//     void fetchMore(const QModelIndex &parent) override;
//     bool canFetchMore(const QModelIndex &parent) const override;

//     void add(albert::Extension*, std::vector<std::shared_ptr<albert::Item>>&&);

//     void add(std::vector<std::pair<albert::Extension*,std::shared_ptr<albert::Item>>>::iterator begin,
//              std::vector<std::pair<albert::Extension*,std::shared_ptr<albert::Item>>>::iterator end);

//     void add(std::vector<std::pair<albert::Extension*,albert::RankItem>>::iterator begin,
//              std::vector<std::pair<albert::Extension*,albert::RankItem>>::iterator end);

//     QAbstractListModel *buildActionsModel(uint i) const;

//     void activate(albert::Query *q, uint i, uint a);

// private:
//     std::vector<std::pair<albert::Extension*, std::shared_ptr<albert::Item>>> items;
//     mutable std::map<std::pair<albert::Extension*,albert::Item*>, QStringList> actionsCache;

// };
