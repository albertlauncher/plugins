// Copyright (C) 2014-2021 Manuel Schneider

#pragma once
#include "albert.h"

class Plugin final
        : public Core::Extension,
          public Core::QueryHandler
{
    Q_OBJECT ALBERT_PLUGIN
public:
    Plugin();
    QString name() const override { return "Template"; }
    void handleQuery(Core::Query*) const override;
    QWidget *widget(QWidget *parent = nullptr) override { return nullptr; }
};
