// Copyright (c) 2022-2025 Manuel Schneider

#pragma once
#include <QPlainTextEdit>
#include <albert/inputhistory.h>

class InputLine : public QPlainTextEdit
{
    Q_OBJECT

public:

    InputLine(QWidget *parent = nullptr);

    const QString& synopsis() const;
    void setSynopsis(const QString&);

    const QString& completion() const;
    void setCompletion(const QString& = {});

    uint triggerLength() const;
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
    QString completion_;
    QString synopsis_;
    QString user_text_;
    uint trigger_length_;
    class TriggerHighlighter;
    TriggerHighlighter *highlighter_;

signals:

    void textEdited();

};


