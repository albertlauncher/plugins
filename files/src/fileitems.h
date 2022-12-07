// Copyright (c) 2022 Manuel Schneider

#pragma once
#include "albert/extensions/queryhandler.h"
#include <QMimeType>
class DirNode;


class AbstractFileItem : public albert::Item
{
public:
    virtual QString name() const = 0;
    virtual QString path() const = 0;
    virtual QString filePath() const = 0;
    virtual const QMimeType &mimeType() const = 0;
    QString id() const override;
    QString text() const override;
    QString subtext() const override;
    QStringList iconUrls() const override;
    QString inputActionText() const override;
    std::vector<albert::Action> actions() const override;
};


class IndexFileItem : public AbstractFileItem
{
public:
    explicit IndexFileItem(const QString &name, const QMimeType &mime, const std::shared_ptr<DirNode> &parent);
    QString name() const override;
    QString path() const override;
    QString filePath() const override;
    const QMimeType &mimeType() const override;
private:
    const QString name_;
    const QMimeType mimetype_;
    const std::shared_ptr<DirNode> parent_;
};


class StandardFile : public AbstractFileItem
{
public:
    StandardFile(QString filePath, QMimeType mimetype, QString completion = {});
    QString name() const override;
    QString path() const override;
    QString filePath() const override;
    const QMimeType &mimeType() const override;
    QString inputActionText() const override;
protected:
    QString name_;
    QString path_;
    QString completion_;
    QMimeType mimetype_;

};

