// Copyright (C) 2014-2018 Manuel Schneider

#pragma once
#include <QDialog>
#include "ui_searchengineeditor.h"
#include "searchengine.h"

namespace Websearch {

class SearchEngineEditor : public QDialog
{
    Q_OBJECT

public:

    explicit SearchEngineEditor(const SearchEngine &searchEngine, QWidget *parent = 0);
    const SearchEngine &searchEngine() { return searchEngine_; }

private:

    SearchEngine searchEngine_;
    Ui::SearchEngineEditor ui;

};

}
