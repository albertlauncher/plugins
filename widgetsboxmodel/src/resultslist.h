// Copyright (c) 2014-2025 Manuel Schneider

#pragma once
#include "resizinglist.h"
class ResultsListDelegate;

class ResultsList : public ResizingList
{
public:

    ResultsList(QWidget *parent = nullptr);
    ~ResultsList();

    QColor textColor() const;
    void setTextColor(QColor);

    QColor subtextColor() const;
    void setSubextColor(QColor);

    uint iconSize() const;
    void setIconSite(uint);

    uint textFontSize() const;
    void setTextFontSize(uint);

    uint subtextFontSize() const;
    void setSubextFontSize(uint);

    uint horizonzalSpacing() const;
    void setHorizonzalSpacing(uint);

    uint verticalSpacing() const;
    void setVerticalSpacing(uint);

    bool debugMode() const;
    void setDebugMode(bool);

private:

    ItemDelegateBase *delegate() const override;
    ResultsListDelegate *delegate_;

};
