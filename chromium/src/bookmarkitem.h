// Copyright (c) 2022-2024 Manuel Schneider

#pragma once
#include <albert/item.h>

class BookmarkItem : public albert::Item
{
public:

    BookmarkItem(const QString &i, const QString &n, const QString &f, const QString &u);

    QString id() const override;
    QString text() const override;
    QString subtext() const override;
    QString inputActionText() const override;
    QStringList iconUrls() const override;
    std::vector<albert::Action> actions() const override;

    const QString id_;
    const QString name_;
    const QString folder_;
    const QString url_;
};
