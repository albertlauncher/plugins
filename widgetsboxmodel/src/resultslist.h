// Copyright (c) 2014-2023 Manuel Schneider

#pragma once
#include "resizinglist.h"

class ResultsList : public ResizingList
{
public:
    ResultsList(QWidget *parent = nullptr);
    void setStyle(const Style*);

private:
    class ResultDelegate;
    ResultDelegate *delegate;

    QFont text_font_;
    QFont subtext_font_;
    QFontMetrics text_font_metrics_;
    QFontMetrics subtext_font_metrics_;
};
