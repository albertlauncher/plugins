// Copyright (C) 2014-2015 Manuel Schneider

#include <QApplication>
#include <QClipboard>
#include <QPointer>
#include <QCryptographicHash>
#include <stdexcept>
#include "configwidget.h"
#include "util/standarditem.h"
#include "extension.h"
using namespace std;
using namespace Core;

namespace {
const QStringList algorithmNames = {
    "MD4",
    "MD5",
    "SHA1",
    "SHA224",
    "SHA256",
    "SHA384",
    "SHA512",
    "SHA3_224",
    "SHA3_256",
    "SHA3_384",
    "SHA3_512"
};
}


class HashGenerator::Private
{
public:
    QPointer<ConfigWidget> widget;
};



/** ***************************************************************************/
HashGenerator::Extension::Extension()
    : Core::Extension("org.albert.extension.hashgenerator"), // Must match the id in metadata
      Core::QueryHandler(Core::Plugin::id()),
      d(new Private) {
    registerQueryHandler(this);
}



/** ***************************************************************************/
HashGenerator::Extension::~Extension() {

}



/** ***************************************************************************/
QWidget *HashGenerator::Extension::widget(QWidget *parent) {
    if (d->widget.isNull()) {
        d->widget = new ConfigWidget(parent);
    }
    return d->widget;
}



/** ***************************************************************************/
QStringList HashGenerator::Extension::triggers() const {
    return {
        "hash ",
        "md4 ",
        "md5 ",
        "sha1 ",
        "sha224 ",
        "sha256 ",
        "sha384 ",
        "sha512 ",
        "sha3_224 ",
        "sha3_256 ",
        "sha3_384 ",
        "sha3_512 "
    };
}


/** ***************************************************************************/
void HashGenerator::Extension::handleQuery(Core::Query * query) const {

    auto buildItem = [](int algorithm, QString string){
        QCryptographicHash hash(static_cast<QCryptographicHash::Algorithm>(algorithm));
        hash.addData(string.toUtf8());
        QByteArray hashString = hash.result().toHex();

        auto item = std::make_shared<StandardItem>(algorithmNames[algorithm]);
        item->setText(QString("%1 of '%2'").arg(algorithmNames[algorithm], string));
        item->setSubtext(hashString);
        item->setIconPath(":hash");
        item->setCompletionString(QString("%1 %2").arg(algorithmNames[algorithm].toLower(), string));
        item->emplaceAction("Copy hash value to clipboard",
                            [=](){ QApplication::clipboard()->setText(QString(hashString));});
        return item;
    };

    if ( query->trigger() == "hash " ) {
        // Output all hashes
        for (int algorithm = 0; algorithm < 11; ++algorithm)
            query->addMatch(buildItem(algorithm, query->string()));
    } else {
        // Output particular hash if name matches
        auto it = std::find(algorithmNames.begin(), algorithmNames.end(),
                            query->trigger().trimmed().toUpper());
        if (it != algorithmNames.end()) {
            int algorithm = static_cast<int>(std::distance(algorithmNames.begin(), it));
            query->addMatch(buildItem(algorithm, query->string()));
        }
    }
}

