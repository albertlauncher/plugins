// Copyright (c) 2014-2025 Manuel Schneider

#pragma once
#include "resizinglist.h"
class ResultsListDelegate;

class ResultsList : public ResizingList
{
public:

    ResultsList(QWidget *parent = nullptr);
    ~ResultsList();

    QColor subtextColor() const;
    void setSubtextColor(QColor);

    QColor selectionSubtextColor() const;
    void setSelectionSubextColor(QColor);

    uint iconSize() const;
    void setIconSite(uint);

    uint subtextFontSize() const;
    void setSubextFontSize(uint);

    uint horizonzalSpacing() const;
    void setHorizonzalSpacing(uint);

    uint verticalSpacing() const;
    void setVerticalSpacing(uint);

private:

    ItemDelegateBase *delegate() const override;
    ResultsListDelegate *delegate_;

};
