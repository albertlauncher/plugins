// Copyright (c) 2024 Manuel Schneider

#pragma once

#include <QString>
#include <albert/export.h>
#include <albert/extension.h>
class QWidget;

class ALBERT_EXPORT Snippets : virtual public albert::Extension
{
public:
    virtual void addSnippet(const QString &text = {}, QWidget *modal_parent = nullptr) const = 0;

protected:
    virtual ~Snippets() = default;

};
