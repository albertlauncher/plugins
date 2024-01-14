// Copyright (c) 2022-2024 Manuel Schneider

#include "albert/albert.h"
#include "albert/extension/queryhandler/standarditem.h"
#include "plugin.h"
#include <QCoreApplication>
#include <QCryptographicHash>
#include <QMetaEnum>
#include <memory>
using namespace albert;
using namespace std;

static int algo_count = QMetaEnum::fromType<QCryptographicHash::Algorithm>().keyCount()-1;

static shared_ptr<Item> buildItem(int algo_index, const QString& string_to_hash)
{
    QString algo_name = QMetaEnum::fromType<QCryptographicHash::Algorithm>().key(algo_index);
    int algo_enum_value = QMetaEnum::fromType<QCryptographicHash::Algorithm>().value(algo_index);

    QCryptographicHash hash(static_cast<QCryptographicHash::Algorithm>(algo_enum_value));
    hash.addData(string_to_hash.toUtf8());
    QByteArray hashString = hash.result().toHex();

    static const auto tr_c = QCoreApplication::translate("buildItem", "Copy");
    static const auto tr_cs = QCoreApplication::translate("buildItem", "Copy short form (8 char)");

    return StandardItem::make(
        algo_name,
        hashString,
        algo_name,
        QStringList({":hash"}),
        {
            {
                "c", tr_c,
                [hashString](){ albert::setClipboardText(QString(hashString)); }
            },
            {
                "cs", tr_cs,
                [hashString](){ albert::setClipboardText(QString(hashString.left(8))); }
            }
        }
    );
};

vector<RankItem> Plugin::handleGlobalQuery(const GlobalQuery *query) const
{
    vector<RankItem> results;
    for (int i = 0; i < algo_count; ++i){
        auto prefix = QString("%1 ").arg(QMetaEnum::fromType<QCryptographicHash::Algorithm>().key(i)).toLower();
        if (query->string().size() >= prefix.size() && query->string().startsWith(prefix, Qt::CaseInsensitive)) {
            QString string_to_hash = query->string().mid(prefix.size());
            results.emplace_back(buildItem(i, string_to_hash), 1.0f);
        }
    }
    return results;
}

void Plugin::handleTriggerQuery(TriggerQuery *query) const
{
    for (int i = 0; i < algo_count; ++i)
        query->add(buildItem(i, query->string()));
}
