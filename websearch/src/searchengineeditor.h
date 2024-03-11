// Copyright (C) 2014-2021 Manuel Schneider

#pragma once
#include "ui_searchengineeditor.h"
#include <QDialog>
#include <QImage>

class SearchEngineEditor : public QDialog
{
    Q_OBJECT
public:
    explicit SearchEngineEditor(const QString &icon_url,
                                const QString &name,
                                const QString &trigger,
                                const QString &url,
                                bool fallback,
                                QWidget *parent);

    std::unique_ptr<QImage> icon_image;
    QString name() const;
    QString trigger() const;
    QString url() const;
    bool fallback() const;

private:
    Ui::SearchEngineEditor ui;
    bool eventFilter(QObject *watched, QEvent *event) override;
};
