// Copyright (C) 2014-2018 Manuel Schneider

#pragma once
#include <QAbstractItemModel>
#include <memory>
#include "core_globals.h"
#include "core/frontend.h"

namespace QmlBoxModel {

class MainWindow;

class FrontendPlugin final : public Core::Frontend
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID ALBERT_FRONTEND_IID FILE "metadata.json")

public:

    FrontendPlugin();
    ~FrontendPlugin();

    bool isVisible() override;
    void setVisible(bool visible) override;

    QString input() override;
    void setInput(const QString&) override;

    void setModel(QAbstractItemModel *) override;

    QWidget* widget(QWidget *parent = nullptr) override;

private:

    std::unique_ptr<MainWindow> mainWindow;
};

}
