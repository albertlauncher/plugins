// Copyright (c) 2022-2024 Manuel Schneider

#pragma once
#include <QPlainTextEdit>
#include <albert/inputhistory.h>

class InputLine : public QPlainTextEdit
{
    Q_OBJECT

public:

    InputLine(QWidget *parent = nullptr);

    void setInputHint(const QString&);
    void setCompletion(const QString&);
    void setTriggerLength(uint);

    bool clear_on_hide;
    bool history_search;

    QString text() const;
    void setText(QString);

    uint fontSize() const;
    void setFontSize(uint);

    void next();
    void previous();

private:

    void paintEvent(QPaintEvent *event) override;
    void hideEvent(QHideEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;

    albert::InputHistory history_;
    QString input_hint_;
    QString completion_;
    QString user_text_;
    QString text_;
    uint trigger_length_;
    class TriggerHighlighter;
    TriggerHighlighter *highlighter_;

signals:

    void textChanged(const QString&);

};


