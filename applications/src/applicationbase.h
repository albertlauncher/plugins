// Copyright (c) 2022-2024 Manuel Schneider

#pragma once
#include "applications.h"
#include <QCoreApplication>
#include <QStringList>
#include <albert/action.h>
#include <albert/item.h>
#include <vector>

class ApplicationBase : public applications::Application, public albert::Item
{
    Q_DECLARE_TR_FUNCTIONS(ApplicationBase)

public:

    QString path() const override final;
    QString id() const override final;
    QString name() const override final;
    QString text() const override final;
    QString inputActionText() const override final;
    std::vector<albert::Action> actions() const override;

    const QStringList &names() const;

protected:

    QString id_;
    QStringList names_;
    QString path_;

};
