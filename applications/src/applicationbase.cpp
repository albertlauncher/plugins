// Copyright (c) 2022-2024 Manuel Schneider

#include "applicationbase.h"
using namespace std;
using namespace albert;

const QStringList &ApplicationBase::names() const { return names_; }

QString ApplicationBase::path() const { return path_; }

QString ApplicationBase::id() const { return id_; }

QString ApplicationBase::name() const { return names_[0]; }

QString ApplicationBase::text() const { return name(); }

QString ApplicationBase::inputActionText() const { return name(); }

vector<Action> ApplicationBase::actions() const
{
    vector<Action> actions;
    actions.emplace_back("launch", tr("Launch application"), [this]{ launch(); });
    return actions;
}
