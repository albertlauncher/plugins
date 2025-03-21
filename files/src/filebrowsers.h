// Copyright (c) 2022-2024 Manuel Schneider

#pragma once
#include <QCoreApplication>
#include <QFileInfoList>
#include <albert/triggerqueryhandler.h>

class FilePathBrowser : public albert::TriggerQueryHandler
{
public:

    FilePathBrowser(bool &matchCaseSensitive, bool &showHidden,
                    bool &sortCaseSensitive, bool &showDirsFirst);
    bool allowTriggerRemap() const override;

protected:

    QFileInfoList listFiles(const QString &filter_path) const;

private:

    bool &match_case_sensitive_;
    bool &show_hidden_;
    bool &sort_case_insensitive_;
    bool &show_dirs_first_;

};

class RootBrowser : public FilePathBrowser
{
    Q_DECLARE_TR_FUNCTIONS(RootBrowser)
public:
    RootBrowser(bool &matchCaseSensitive, bool &showHidden,
                bool &sortCaseSensitive, bool &showDirsFirst);
    QString id() const override;
    QString name() const override;
    QString description() const override;
    QString defaultTrigger() const override;
    void handleTriggerQuery(albert::Query &) override;
};

class HomeBrowser : public FilePathBrowser
{
    Q_DECLARE_TR_FUNCTIONS(HomeBrowser)
public:
    HomeBrowser(bool &matchCaseSensitive, bool &showHidden,
                bool &sortCaseSensitive, bool &showDirsFirst);
    QString id() const override;
    QString name() const override;
    QString description() const override;
    QString defaultTrigger() const override;
    void handleTriggerQuery(albert::Query &) override;
};
