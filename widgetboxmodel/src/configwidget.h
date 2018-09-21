// Copyright (C) 2014-2018 Manuel Schneider

#pragma once
#include <QWidget>
#include <memory>

namespace WidgetBoxModel {

class FrontendWidget;

class ConfigWidget final : public QWidget
{
    Q_OBJECT

    class Private;

public:

    ConfigWidget(FrontendWidget *frontend, QWidget *parent);
    ~ConfigWidget();

private:

    std::unique_ptr<Private> d;

};

}
