// Copyright (c) 2022-2024 Manuel Schneider

#pragma once
#include "applicationbase.h"
#include <QString>
#include <QUrl>
#include <albert/item.h>

class Application : public ApplicationBase
{
public:

    struct ParseOptions
    {
        bool ignore_show_in_keys;
        bool use_exec;
        bool use_generic_name;
        bool use_keywords;
        bool use_non_localized_name;
    };

    Application(const QString &id, const QString &path, ParseOptions po);
    Application(const Application &) = default;

    QString subtext() const override final;
    QStringList iconUrls() const override final;
    void launch() const override final;
    std::vector<albert::Action> actions() const override final;

    const QStringList &exec() const;

    bool isTerminal() const;

protected:

    void launchExec(const QStringList &exec, QUrl url, const QString &working_dir) const;

    struct DesktopAction {
        const Application &application;
        QString id_;
        QString name_;
        QStringList exec_;
        void launch() const;
    };

private:

    QStringList fieldCodesExpanded(const QStringList &exec, QUrl url = {}) const;

    QString description_;
    QString icon_;
    QStringList exec_;
    QString working_dir_;
    std::vector<DesktopAction> desktop_actions_;
    bool term_ = false;
    bool is_terminal_ = false;

};
