// Copyright (c) 2022 Manuel Schneider

#include "albert.h"
#include "plugin.h"
#include <QCryptographicHash>
#include <QMetaEnum>
#include <memory>
using namespace std;
using namespace albert;

static int algo_count = QMetaEnum::fromType<QCryptographicHash::Algorithm>().keyCount();

static shared_ptr<Item> buildItem(int algo_index, const QString& string_to_hash)
{
    QString algo_name = QMetaEnum::fromType<QCryptographicHash::Algorithm>().key(algo_index);
    int algo_enum_value = QMetaEnum::fromType<QCryptographicHash::Algorithm>().value(algo_index);

    QCryptographicHash hash(static_cast<QCryptographicHash::Algorithm>(algo_enum_value));
    hash.addData(string_to_hash.toUtf8());
    QByteArray hashString = hash.result().toHex();

    return StandardItem::make(
            algo_name,
            hashString,
            QString("%1 of '%2'").arg(algo_name, string_to_hash),
            QStringList({":hash"}),
            {
                {"clip", "Copy", [hashString](){ albert::setClipboardText(QString(hashString)); }},
                {"clip-short", "Copy short form (8 char)", [hashString](){ albert::setClipboardText(QString(hashString.left(8))); }}
            }
    );
};

vector<RankItem> Plugin::handleGlobalQuery(const GlobalQuery &query) const
{
    vector<RankItem> results;
    for (int i = 0; i < algo_count; ++i){
        auto prefix = QString("%1 ").arg(QMetaEnum::fromType<QCryptographicHash::Algorithm>().key(i)).toLower();
        if (query.string().size() >= prefix.size() && query.string().startsWith(prefix, Qt::CaseInsensitive)) {
            QString string_to_hash = query.string().mid(prefix.size());
            results.emplace_back(buildItem(i, string_to_hash), RankItem::MAX_SCORE);
        }
    }
    return results;
}

void Plugin::handleTriggerQuery(TriggerQuery &query) const
{
    for (int i = 0; i < algo_count; ++i)
        query.add(buildItem(i, query.string()));
}
