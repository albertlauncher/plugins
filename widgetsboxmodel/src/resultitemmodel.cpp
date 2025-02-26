// Copyright (c) 2022-2024 Manuel Schneider

#include <albert/extension.h>
#include <albert/frontend.h>
#include <albert/item.h>
#include <albert/query.h>
#include <albert/logging.h>
#include <albert/rankitem.h>
#include "resultitemmodel.h"
#include <QStringListModel>
#include <QTimer>
using namespace albert;
using namespace std;


// -------------------------------------------------------------------------------------------------

ResultItemsModel::ResultItemsModel(Query &query):
    query_(query)
{
    connect(&query, &Query::matchesAboutToBeAdded, this, [&, this](uint count){
        Q_ASSERT(count > 0);
        beginInsertRows({}, query.matches().size(), query.matches().size() + count - 1);
    });

    connect(&query, &Query::matchesAdded, this, [this] { endInsertRows(); });
}

QHash<int, QByteArray> ResultItemsModel::roleNames() const
{
    static QHash<int, QByteArray> qml_role_names = {
        {(int)ItemRoles::TextRole, "itemText"},
        {(int)ItemRoles::SubTextRole, "itemSubText"},
        {(int)ItemRoles::InputActionRole, "itemInputAction"},
        {(int)ItemRoles::IconUrlsRole, "itemIconUrls"},
        {(int)ItemRoles::ActionsListRole, "itemActionsList"},
        {(int)ItemRoles::ActivateActionRole, "itemActionActivate"}
    };
    return qml_role_names;
}

QVariant ResultItemsModel::getResultItemData(const ResultItem &result_item, int role) const
{
    const auto &[extension, item] = result_item;

    switch (role) {
        case (int)ItemRoles::TextRole:
        {
            QString text = item->text();
            text.replace('\n', ' ');
            return text;
        }
        case (int)ItemRoles::SubTextRole:
        {
            QString text = item->subtext();
            text.replace('\n', ' ');
            return text;
        }
        case Qt::ToolTipRole:
            return QString("%1\n%2").arg(item->text(), item->subtext());

        case (int)ItemRoles::InputActionRole:
            return item->inputActionText();

        case (int)ItemRoles::IconUrlsRole:
            return item->iconUrls();

        case (int)ItemRoles::ActionsListRole:
        {
            if (auto it = actions_cache_.find(&result_item);
                it != actions_cache_.end())
                return it->second;

            QStringList action_names;
            for (const auto &action : item->actions())
                action_names << action.text;

            // actions_cache_.emplace(make_pair(extension, item.get()), actions_cache_);

            return action_names;
        }
    }
    return {};
}


// -------------------------------------------------------------------------------------------------


int MatchItemsModel::rowCount(const QModelIndex &) const
{ return (int)query_.matches().size(); }

bool MatchItemsModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (index.isValid() && role == (int)ItemRoles::ActivateActionRole)
        return query_.activateMatch(index.row(), value.toUInt());
    else
        return false;
}

QVariant MatchItemsModel::data(const QModelIndex &index, int role) const
{
    if (index.isValid())
        return getResultItemData(query_.matches().at(index.row()), role);
    return {};
}


// -------------------------------------------------------------------------------------------------

int FallbackItemsModel::rowCount(const QModelIndex &) const
{ return (int)query_.fallbacks().size(); }

bool FallbackItemsModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (index.isValid() && role == (int)ItemRoles::ActivateActionRole)
        return query_.activateFallback(index.row(), value.toUInt());
    else
        return false;
}

QVariant FallbackItemsModel::data(const QModelIndex &index, int role) const
{
    if (index.isValid())
        return getResultItemData(query_.fallbacks().at(index.row()), role);
    return {};
}















// void ItemsModel::add(Extension *extension, vector<shared_ptr<Item>> &&itemvec)
// {
//     if (itemvec.empty())
//         return;

//     beginInsertRows(QModelIndex(), (int)items.size(), (int)(items.size()+itemvec.size()-1));
//     items.reserve(items.size()+itemvec.size());
//     for (auto &&item : itemvec)
//         items.emplace_back(extension, ::move(item));
//     endInsertRows();
// }

// void ItemsModel::add(vector<pair<Extension*, shared_ptr<Item>>>::iterator begin,
//                      vector<pair<Extension*, shared_ptr<Item>>>::iterator end)
// {
//     if (begin == end)
//         return;

//     items.reserve(items.size()+(size_t)(end-begin));

//     beginInsertRows(QModelIndex(), (int)items.size(), (int)(items.size())+(int)(end-begin)-1);

//     items.insert(items.end(), make_move_iterator(begin), make_move_iterator(end));

//     endInsertRows();

// }

// void ItemsModel::add(vector<pair<Extension*,RankItem>>::iterator begin,
//                      vector<pair<Extension*,RankItem>>::iterator end)
// {
//     if (begin == end)
//         return;

//     items.reserve(items.size()+(size_t)(end-begin));

//     beginInsertRows(QModelIndex(), (int)items.size(), (int)(items.size())+(int)(end-begin)-1);

//     for (auto it = begin; it != end; ++it)
//         items.emplace_back(it->first, ::move(it->second.item));

//     endInsertRows();
// }

// QAbstractListModel *ItemsModel::buildActionsModel(uint i) const
// {
//     QStringList l;
//     for (const auto &a : items[i].second->actions())
//         l << a.text;
//     return new QStringListModel(l);
// }
