// Copyright (c) 2022-2024 Manuel Schneider

#pragma once
#include "applicationbase.h"
#include <QCoreApplication>
#include <QStringList>

class Application : public ApplicationBase
{
    Q_DECLARE_TR_FUNCTIONS(Application)

public:

    Application(const QString &path, bool use_non_localized_name);
    Application(const Application &) = default;

    QString subtext() const override;
    QStringList iconUrls() const override;
    void launch() const override;

};
