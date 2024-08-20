// Copyright (c) 2022-2024 Manuel Schneider

#pragma once
#include <albert/item.h>
class Docset;

class DocItem : public albert::Item
{
public:

    DocItem(const Docset &ds, const QString &t, const QString &n, const QString &p, const QString &a);

    QString id() const override;
    QString text() const override;
    QString subtext() const override;
    QStringList iconUrls() const override;
    QString inputActionText() const override;
    std::vector<albert::Action> actions() const override;

private:

    void open() const;

    const Docset &docset;
    const QString type;
    const QString name;
    const QString path;
    const QString anchor;
};
