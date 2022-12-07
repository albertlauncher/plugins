// Copyright (c) 2022 Manuel Schneider

#pragma once
#include <QObject>
#include <memory>
#include "albert.h"


class Plugin:
        public albert::NativePlugin,
        public albert::QueryHandler
{
    Q_OBJECT ALBERT_PLUGIN

public:
    Plugin();
    ~Plugin();

    void setupSession() override;
    void handleQuery(albert::Query *query) const override;

private:

    struct Private;
    std::unique_ptr<Private> d;

};
