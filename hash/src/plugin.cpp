// Copyright (c) 2022 Manuel Schneider

#include "albert.h"
#include "plugin.h"
#include <QCryptographicHash>
#include <QMetaEnum>
#include <memory>
using namespace std;
using namespace albert;

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
            Actions{
                {"clip","Copy to clipboard", [hashString](){ albert::setClipboardText(QString(hashString)); }}
            }
    );
};

void ::Plugin::handleQuery(Query &query) const
{
    QString prefix = "hash ";
    int algo_count = QMetaEnum::fromType<QCryptographicHash::Algorithm>().keyCount();
    if (query.string().size() > prefix.size() && query.string().toLower().startsWith(prefix)) {
        QString string_to_hash = query.string().mid(prefix.size());
        for (int i = 0; i < algo_count; ++i)
            query.add(buildItem(i, string_to_hash));
    } else {
        for (int i = 0; i < algo_count; ++i){
            prefix = QString("%1 ").arg(QMetaEnum::fromType<QCryptographicHash::Algorithm>().key(i)).toLower();
            if (query.string().size() > prefix.size() && query.string().toLower().startsWith(prefix)) {
                QString string_to_hash = query.string().mid(prefix.size());
                query.add(buildItem(i, string_to_hash));
            }
        }
    }
}
